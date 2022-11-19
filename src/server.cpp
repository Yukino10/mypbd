//
// Created by yuki on 22-9-30.
//
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
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// A server to receive EchoRequest and send back EchoResponse.

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include "data.pb.h"
#include "myDataBase.cpp"
#include "qps.cpp"

DEFINE_bool(echo_attachment, true, "Echo attachment as well");
//DEFINE_int32(port, 8000, "TCP Port of this server");
DEFINE_string(listen_addr, "", "Server listen address, may be IPV4/IPV6/UDS."
                               " If this is set, the flag port will be ignored");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
                                 "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
                              "(waiting for client to close connection before server stops)");

// Your implementation of example::EchoService
// Notice that implementing brpc::Describable grants the ability to put
// additional information in /status.
qps rpcQps;
class EchoServiceImpl : public data::EchoService {
public:
    int state = 0;
    myDataBase* dataBase;
public:
    EchoServiceImpl(myDataBase *dataBase){
        this->dataBase = dataBase;
    }
    virtual ~EchoServiceImpl() {};
    virtual void Echo(google::protobuf::RpcController* cntl_base,
                      const data::EchoRequest* request,
                      data::EchoResponse* response,
                      google::protobuf::Closure* done) {
        // This object helps you to call done->Run() in RAII style. If you need
        // to process the request asynchronously, pass done_guard.release().
        brpc::ClosureGuard done_guard(done);

        brpc::Controller* cntl =
                static_cast<brpc::Controller*>(cntl_base);

        // The purpose of following logs is to help you to understand
        // how clients interact with servers more intuitively. You should
        // remove these logs in performance-sensitive servers.
//            LOG(INFO) << "Received request[log_id=" << cntl->log_id()
//                      << "] from " << cntl->remote_side()
//                      << " to " << cntl->local_side()
//                      << ": " << request->column_key()
//                      << " (attached=" << cntl->request_attachment() << ")";

        // Fill response.
        rpcQps.increase(request->select_column_size());
        if(request->select_column()[0] == -1){
            response->add_len(state);
            response->add_response("");
        }
        else {
            for(int i = 0; i < request->select_column_size(); i++){
                char res[256000];
                int len = dataBase->read(request->select_column(i), request->where_column(i), request->column_key(i).c_str(), request->column_key().size(), res);
                string sRes;
                sRes.assign(res, len * ((request->select_column(i) == 0 || request->select_column(i) == 3) ? 8 : 128));
                response->add_response(sRes);
                response->add_len(len);
            }
        }
        // You can compress the response by setting Controller, but be aware
        // that compression may be costly, evaluate before turning on.
        // cntl->set_response_compress_type(brpc::COMPRESS_TYPE_GZIP);

        if (FLAGS_echo_attachment) {
            // Set attachment which is wired to network directly instead of
            // being serialized into protobuf messages.
            cntl->response_attachment().append(cntl->request_attachment());
        }
    }
};
// namespace example

class server{
public:
    brpc::Server service;
    // Instance of your service.
    EchoServiceImpl* echo_service_impl;
    butil::EndPoint point;
    // Add the service into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    brpc::ServerOptions options;
public:
    server(int32_t port, myDataBase* dataBase){
        options.num_threads = 50;
        echo_service_impl = new EchoServiceImpl(dataBase);
        if (service.AddService(echo_service_impl,
                              brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
            LOG(ERROR) << "Fail to add service";
        }
        if (!FLAGS_listen_addr.empty()) {
            if (butil::str2endpoint(FLAGS_listen_addr.c_str(), &point) < 0) {
                LOG(ERROR) << "Invalid listen address:" << FLAGS_listen_addr;
            }
        } else {
            point = butil::EndPoint(butil::IP_ANY, port);
        }
    }
    // Start the server.
    void setState(int state){
        echo_service_impl->state = state;
    }
    void run(){
        options.idle_timeout_sec = FLAGS_idle_timeout_s;
        if (service.Start(point, &options) != 0) {
            LOG(ERROR) << "Fail to start EchoServer";
        }
    }
    void Quit(){
        service.Stop(4000);
        // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    }
};
