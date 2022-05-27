cmake_minimum_required(VERSION 3.12.4)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project(DDSHelloWorld)

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

# Set C++11
include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(SUPPORTS_CXX11)
        add_compile_options(-std=c++11)
    else()
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring HelloWorld publisher/subscriber example...")
file(GLOB DDS_HELLOWORLD_SOURCES_CXX "src/helloworld/*.cxx")

add_executable(DDSHelloWorldPublisher src/helloworld/HelloWorldPublisher.cpp ${DDS_HELLOWORLD_SOURCES_CXX})
target_link_libraries(DDSHelloWorldPublisher fastrtps fastcdr)

add_executable(DDSHelloWorldSubscriber src/helloworld/HelloWorldSubscriber.cpp ${DDS_HELLOWORLD_SOURCES_CXX})
target_link_libraries(DDSHelloWorldSubscriber fastrtps fastcdr)

file(GLOB DDS_OWNERSHIP_SOURCES "src/ownership/*.cxx")
set(DDS_OWNERSHIP_SOURCES_HEADER "src/ownership")

add_executable(DDSPublisherOwnership src/ownership/DDSPublisher_ownership.cpp ${DDS_OWNERSHIP_SOURCES})
target_include_directories(DDSPublisherOwnership PRIVATE ${DDS_OWNERSHIP_SOURCES_HEADER})
target_link_libraries(DDSPublisherOwnership fastrtps fastcdr)

add_executable(DDSSubscriberOwnership src/ownership/DDSSubscriber_ownership.cpp ${DDS_OWNERSHIP_SOURCES})
target_include_directories(DDSSubscriberOwnership PRIVATE ${DDS_OWNERSHIP_SOURCES_HEADER})
target_link_libraries(DDSSubscriberOwnership fastrtps fastcdr)