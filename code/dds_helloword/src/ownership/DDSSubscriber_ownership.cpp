// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file HelloWorldSubscriber_ownership.cpp
 *
 */

#include "ShapeTypePubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastrtps::rtps;

class DDSSubscriberOwnership
{
private:
    DomainParticipant *participant_;

    Subscriber *subscriber_;

    DataReader *reader_;

    Topic *topic_;

    TypeSupport type_;

    std::map<eprosima::fastrtps::rtps::GUID_t, uint32_t> m_ownership_strength_map;

    uint32_t highestOwnership_strength_ = 0;

    class DPListener : public eprosima::fastdds::dds::DomainParticipantListener
    {
    public:
        DPListener(DDSSubscriberOwnership *parent) : parent_(parent) {}

        void on_publisher_discovery(
            DomainParticipant *dp,
            eprosima::fastrtps::rtps::WriterDiscoveryInfo &&info)
        {
            switch (info.status)
            {
            case WriterDiscoveryInfo::DISCOVERED_WRITER:
            case WriterDiscoveryInfo::CHANGED_QOS_WRITER:
            {
                std::cout << "PUBLISHER guid=" << info.info.guid() << ",ownershipStrength=" << info.info.m_qos.m_ownershipStrength.value << std::endl;
                parent_->m_ownership_strength_map[info.info.guid()] = info.info.m_qos.m_ownershipStrength.value;
                parent_->calculateHighestStrength();
                break;
            }
            case WriterDiscoveryInfo::REMOVED_WRITER:
            {
                parent_->m_ownership_strength_map.erase(info.info.guid());
                parent_->calculateHighestStrength();
                break;
            }
            }
        }
        DDSSubscriberOwnership *parent_;
    } *dpListener_;

    class SubListener : public DataReaderListener
    {
    public:
        SubListener(DDSSubscriberOwnership *parent) : parent_(parent), samples_(0)
        {
        }

        ~SubListener() override
        {
        }

        void on_subscription_matched(
            DataReader *,
            const SubscriptionMatchedStatus &info) override
        {
            if (info.current_count_change == 1)
            {
                std::cout << "Subscriber matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                std::cout << "Subscriber unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                          << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
            }
        }

        void on_data_available(
            DataReader *reader) override
        {
            SampleInfo info;
            if (reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK)
            {
                GUID_t guid = eprosima::fastrtps::rtps::iHandle2GUID(info.publication_handle);
                // user-level to implement ownership and ownership_strength
                if (!parent_->isHighestStrength(guid))
                    return;
                // if (info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
                {
                    uint32_t strength = parent_->m_ownership_strength_map[guid];
                    samples_++;
                    std::cout << "Strength: " << strength << " Color: " << hello_.color() << " with size: " << hello_.shapesize() << " with x: " << hello_.x()
                              << " with y: " << hello_.y()
                              << " RECEIVED." << std::endl;
                }
            }
        }

        ShapeType hello_;
        std::atomic_int samples_;
        DDSSubscriberOwnership *parent_;
    } * listener_;
    // DPListener *dpListener_;

public:
    DDSSubscriberOwnership()
        : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr), type_(new ShapeTypePubSubType())
    {
    }

    virtual ~DDSSubscriberOwnership()
    {
        if (reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    //! Initialize the subscriber
    bool init()
    {
        // Create the participant
        dpListener_ = new DPListener(this);
        DomainParticipantQos participantQos;
        participantQos.name("Participant_subscriber");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos, dpListener_, eprosima::fastdds::dds::StatusMask::none());

        if (participant_ == nullptr)
        {
            return false;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the subscriptions Topic
        topic_ = participant_->create_topic("ShapeTypeTopic", "ShapeType", TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            return false;
        }
        listener_ = new SubListener(this);
        // Create the Subscriber
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

        if (subscriber_ == nullptr)
        {
            return false;
        }
        DataReaderQos drQos;
        subscriber_->get_default_datareader_qos(drQos);
        drQos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;
        drQos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        // Create the DataReader
        reader_ = subscriber_->create_datareader(topic_, drQos, listener_);

        if (reader_ == nullptr)
        {
            std::cout << "create reader failed" << std::endl;
            return false;
        }

        return true;
    }

    //! Run the Subscriber
    void run(
        uint32_t samples)
    {
        while (listener_->samples_ < samples)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    bool isHighestStrength(GUID_t &guid)
    {
        uint32_t strength = m_ownership_strength_map[guid];
        if (strength < highestOwnership_strength_)
        {
            return false;
        }
        return true;
    }

    void calculateHighestStrength()
    {
        highestOwnership_strength_ = 0;
        for (const auto &n : m_ownership_strength_map)
        {

            if (n.second > highestOwnership_strength_)
            {
                highestOwnership_strength_ = n.second;
            }
        }
    }
};

int main(
    int argc,
    char **argv)
{
    std::cout << "Starting subscriber." << std::endl;
    int samples = 200;

    DDSSubscriberOwnership *mysub = new DDSSubscriberOwnership();
    if (mysub->init())
    {
        mysub->run(static_cast<uint32_t>(samples));
    }

    delete mysub;
    return 0;
}
