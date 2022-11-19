//
// Created by yuki on 22-9-30.
//

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <butil/time.h>
#include <brpc/channel.h>
#include "data.pb.h"
#include <utility>
#include "params.h"
using namespace std;

DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "single", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "127.0.0.1:8000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");
class reqData{
public:
    int select_column, where_column, column_key_len;
    char* column_key;
    promise<pair<int, string>>* promiseQ;
public:
    reqData(int select_column, int where_column, char* column_key, int column_key_len, promise<pair<int, string>>* promiseQ){
        this->select_column = select_column;
        this->where_column = where_column;
        this->column_key_len = column_key_len;
        this->column_key = column_key;
        this->promiseQ = promiseQ;
    }
};
void HandleEchoResponse(brpc::Controller* cntl, data::EchoResponse* response, vector<promise<pair<int, string>>*>* myPromise) {
    // std::unique_ptr makes sure cntl/response will be deleted before returning.
    std::unique_ptr<brpc::Controller> cntl_guard(cntl);
    std::unique_ptr<data::EchoResponse> response_guard(response);
    if (cntl->Failed()) {
        LOG(WARNING) << "Fail to send EchoRequest, " << cntl->ErrorText();
        for(auto it : *myPromise){
            it->set_value({-1, ""});
        }
        return;
    }
    for(int i = 0; i < (*myPromise).size(); i++){
        (*myPromise)[i]->set_value({(*response_guard).len(i), (*response_guard).response(i)});
    }
    delete myPromise;
}
class client{
public:
    brpc::Channel channel;
    brpc::ChannelOptions options;
    // Initialize the channel, NULL means using default options.
    data::EchoService_Stub* stub;
    string server;
public:
    client(string server){
        // A Channel represents a communication line to a Server. Notice that
        // Channel is thread-safe and can be shared by all threads in your program.

        // Initialize the channel, NULL means using default options.
        this->server = server;
        options.protocol = FLAGS_protocol;
        options.connection_type = FLAGS_connection_type;
        options.timeout_ms = FLAGS_timeout_ms/*milliseconds*/;
        options.max_retry = FLAGS_max_retry;
    }
    bool init(){
        if (channel.Init(server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0) {
            LOG(ERROR) << "Fail to initialize channel" << endl;
            return 0;
        }
        // Normally, you should not call a Channel directly, but instead construct
        // a stub Service wrapping it. stub can be shared by all threads as well.
        stub = new data::EchoService_Stub(&channel);
        return 1;
    }
    int call(int select_column){
        data::EchoRequest request;
        data::EchoResponse response;
        brpc::Controller cntl;
        request.add_select_column(select_column);
        // Set attachment which is wired to network directly instead of
        // being serialized into protobuf messages.
        cntl.request_attachment().append(FLAGS_attachment);
        // Because `done'(last parameter) is NULL, this function waits until
        // the response comes back or error occurs(including timedout).
        stub->Echo(&cntl, &request, &response, NULL);
        if (!cntl.Failed()) {
            //cout << "attached=" << cntl.response_attachment()<< " latency=" << cntl.latency_us() << "us";
            return response.len(0);
        }
        else {
            LOG(WARNING) << cntl.ErrorText();
            return -1;
        }
    }
    void asyncCall(reqData** reqQueue, int l, int r){
        data::EchoResponse* response = new data::EchoResponse();
        brpc::Controller* cntl = new brpc::Controller();

        // Notice that you don't have to new request, which can be modified
        // or destroyed just after stub.Echo is called.
        data::EchoRequest request;
        vector<promise<pair<int, string>>*>* myPromise= new vector<promise<pair<int, string>>*>;
        while(l != r){
            request.add_select_column(reqQueue[l]->select_column);
            request.add_where_column(reqQueue[l]->where_column);
            string column;
            column.assign(reqQueue[l]->column_key, reqQueue[l]->column_key_len);
            request.add_column_key(column);
            myPromise->push_back(reqQueue[l]->promiseQ);
            delete reqQueue[l];
            l++;
            l %= TASK_QUEUE_SIZE;
        }
        // Set attachment which is wired to network directly instead of
        // being serialized into protobuf messages.
        cntl->request_attachment().append("foo");


        // We use protobuf utility `NewCallback' to create a closure object
        // that will call our callback `HandleEchoResponse'. This closure
        // will automatically delete itself after being called once
        google::protobuf::Closure* done = brpc::NewCallback(
                &HandleEchoResponse, cntl, response, myPromise);
        stub->Echo(cntl, &request, response, done);
    }
};
