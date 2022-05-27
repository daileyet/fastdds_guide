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
 * @file HelloWorldPublisher_ownership.cpp
 *
 */

#include "ShapeTypePubSubTypes.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <signal.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::types;

class DDSPublisherOwnership
{
private:
    ShapeType *hello_;

    DomainParticipant *participant_;

    Publisher *publisher_;

    Topic *topic_;

    DataWriter *writer_;

    TypeSupport type_;

    int ownership_strength_;

    class PubListener : public DataWriterListener
    {
    public:
        PubListener()
            : matched_(0)
        {
        }

        ~PubListener() override
        {
        }

        void on_publication_matched(
            DataWriter *,
            const PublicationMatchedStatus &info) override
        {
            if (info.current_count_change == 1)
            {
                matched_ = info.total_count;
                std::cout << "Publisher matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                matched_ = info.total_count;
                std::cout << "Publisher unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                          << " is not a valid value for PublicationMatchedStatus current count change." << std::endl;
            }
        }

        std::atomic_int matched_;

    } listener_;

public:
    DDSPublisherOwnership(int strength)
        : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr), type_(new ShapeTypePubSubType()), ownership_strength_(strength)
    {
    }

    virtual ~DDSPublisherOwnership()
    {
        if (writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }
        if (publisher_ != nullptr)
        {
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    //! Initialize the publisher
    bool init()
    {
        // Create the participant
        DomainParticipantQos participantQos;
        participantQos.name("Participant_publisher" + std::to_string(ownership_strength_));
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

        if (participant_ == nullptr)
        {
            return false;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the publications Topic
        topic_ = participant_->create_topic("ShapeTypeTopic", "ShapeType", TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr)
        {
            return false;
        }
        hello_ = new ShapeType();
        hello_->color("RED");

        // TopicQos tQos = topic_->get_qos();
        // tQos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;
        // topic_->set_qos(tQos);

        // Create the Publisher
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

        if (publisher_ == nullptr)
        {
            return false;
        }

        DataWriterQos dwQos;
        publisher_->get_default_datawriter_qos(dwQos);
        // dwQos.reliable_writer_qos().times.heartbeatPeriod.seconds = 0;
        // dwQos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 500000000;
        // DataWriterQos dwQos = DATAWRITER_QOS_DEFAULT;
        dwQos.ownership().kind = EXCLUSIVE_OWNERSHIP_QOS;
        dwQos.ownership_strength().value = ownership_strength_;
        dwQos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        // Create the DataWriter
        writer_ = publisher_->create_datawriter(topic_, dwQos, &listener_);

        if (writer_ == nullptr)
        {
            std::cout << "create writer failed" << std::endl;
            return false;
        }
        
        return true;
    }

    //! Send a publication
    bool publish()
    {
        if (listener_.matched_ > 0)
        {
            hello_->x(1);
            hello_->y(1);
            hello_->shapesize(ownership_strength_);
            writer_->write(hello_);
            return true;
        }
        return false;
    }

    //! Run the Publisher
    void run(
        uint32_t samples)
    {
        uint32_t samples_sent = 0;
        std::cout << "Start to publish data " << samples << " times." << std::endl;
        while (samples_sent < samples)
        {

            if (publish())
            {
                samples_sent++;
                std::cout << "Color: " << hello_->color() << " with color: " << hello_->shapesize() << " with x: " << hello_->x()
                          << " with y: " << hello_->y()
                          << " SENT." << std::endl;
            }
            else
            {
                // std::cout << "warning:no matched subscriber" << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
};

DDSPublisherOwnership *mypub;

void signal_callback_handler(int signum)
{
    std::cout << "Caught terminate signal " << signum << std::endl;
    if (mypub)
    {
        delete mypub;
    }
    // Terminate program
    exit(signum);
}

int main(
    int argc,
    char *argv[])
{
    signal(SIGINT, signal_callback_handler);
    int samples = 100;
    int strength = 0;
    if (argc >= 2)
    {
        try
        {
            strength = std::stoi(argv[1]);
        }
        catch (...)
        {
            std::cout << "Got exception for parameter strength";
        }
    }
    if (argc >= 3)
    {
        try
        {
            samples = std::stoi(argv[2]);
        }
        catch (...)
        {
            std::cout << "Got exception for parameter sample";
        }
    }
    std::cout << "Starting publisher with ownership strength=" << strength << std::endl;
    mypub = new DDSPublisherOwnership(strength);
    if (mypub->init())
    {
        mypub->run(static_cast<uint32_t>(samples));
    }

    delete mypub;
    return 0;
}
