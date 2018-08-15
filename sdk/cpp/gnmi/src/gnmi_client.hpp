/// YANG Development Kit
// Copyright 2016 Cisco Systems. All rights reserved
//
////////////////////////////////////////////////////////////////
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//
//////////////////////////////////////////////////////////////////

#ifndef _YDK_GNMI_CLIENT_H_
#define _YDK_GNMI_CLIENT_H_
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/grpc++.h>
#include "gnmi.grpc.pb.h"
#include "gnmi.pb.h"

#include <ydk/types.hpp>

using namespace gnmi;

using gnmi::gNMI;
using gnmi::CapabilityRequest;
using gnmi::CapabilityResponse;
using grpc::Channel;
using grpc::ClientContext;
using gnmi::GetRequest;
using gnmi::GetResponse;
using gnmi::SetRequest;
using gnmi::SetResponse;
using gnmi::SubscribeRequest;
using gnmi::SubscribeResponse;
using grpc::Status;

namespace ydk 
{

class Entity;

struct GnmiClientRequest {
	std::string alias;
	std::string payload;
	gnmi::Path* path;
	std::string type;
	std::string operation;
};

struct GnmiClientModelData {
	std::string name;
	std::string organization;
	std::string version;
};

struct GnmiClientCapabilityResponse {
    std::string gnmi_version;
    std::vector<GnmiClientModelData>  supported_models;
    std::vector<std::string> supported_encodings;
};

struct GnmiClientSubscription {
    gnmi::Path* path;
    std::string subscription_mode;
    uint64 sample_interval;
    bool suppress_redundant;
    uint64 heartbeat_interval;
};

class gNMIClient;

void poll_thread_callback_control(gNMIClient* client, std::function<bool(const std::string & response)> poll_func);
void poll_thread_cin_control(gNMIClient* client, std::function<bool(const std::string & response)> poll_func);

class gNMIClient
{
  public:
    typedef struct PathPrefixValueFlags
    {
        bool path_has_value;
        bool prefix_has_value;
    } PathPrefixValueFlags;

    gNMIClient(std::shared_ptr<Channel> channel, const std::string & username, const std::string & password);
    gNMIClient(std::shared_ptr<Channel> channel);
    ~gNMIClient();

    int connect();

    std::vector<std::string> execute_get_operation(const std::vector<GnmiClientRequest> get_request_list, const std::string& operation);

    bool execute_set_operation(const std::vector<GnmiClientRequest> get_request_list);

    void execute_subscribe_operation(std::vector<GnmiClientSubscription> subscription_list,
                                     uint32 qos, const std::string & mode,
                                     std::function<void(const std::string & response)> out_func,
                                     std::function<bool(const std::string & response)> poll_func);
    void send_poll_request();
    std::vector<std::string> get_capabilities();
    GnmiClientCapabilityResponse execute_get_capabilities();

    std::shared_ptr<grpc::ClientReaderWriter<gnmi::SubscribeRequest, ::gnmi::SubscribeResponse>> client_reader_writer;

    inline void set_poll_thread_control_function(
    		std::function<void(gNMIClient*, std::function<bool(const std::string&)>)> thread_func)
    {
        poll_thread_control = thread_func;
    };

    inline std::string get_last_subscribe_response()
    {
        return last_subscribe_response;
    }

  private:

    void parse_capabilities_modeldata(::gnmi::CapabilityResponse* response);
    void parse_capabilities(::gnmi::CapabilityResponse* response);

    std::vector<std::string> execute_get_payload(const ::gnmi::GetRequest& request, ::gnmi::GetResponse* response);
    bool execute_set_payload(const ::gnmi::SetRequest& request, ::gnmi::SetResponse* response);

    std::vector<std::string> capabilities;
    std::unique_ptr<gNMI::Stub> stub_;
    std::string username;
    std::string password;
    bool is_secure;

    std::string last_subscribe_response;
    std::function<void(gNMIClient* client, std::function<bool(const std::string & response)>)> poll_thread_control;
};
}

#endif /* _YDK_GNMI_CLIENT_H_ */
