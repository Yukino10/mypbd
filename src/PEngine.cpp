//
// Created by yuki on 22-7-24.
//
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include "list"
#include "queue"
#include "taskQueue.cpp"
#include "server.cpp"
using namespace std;
using namespace std::chrono;
milliseconds now() {
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}

class PEngine {
public:
    milliseconds start;
    myDataBase *dataBase;
    int other_pc_num;
    vector<string>other_ip;
    server* rpcServer;
    client* rpcClient[3];
    taskQueue* myTaskQueue[3];
    int ok[5] = {0};
public:

    PEngine(string &ape_dir, string &ssd_dir, string &host_ip, int &host_port, vector<string>&other_ip, vector<int>&other_port) {
        start = now();
        dataBase = new myDataBase(ape_dir, ssd_dir);
        int rank = 0;
        for(auto it : other_ip)if(host_ip > it)rank++;
        while(rank--){
            string now_ip = other_ip[0];
            int now_port = other_port[0];
            other_ip.erase(other_ip.begin());
            other_port.erase(other_port.begin());
            other_ip.push_back(now_ip);
            other_port.push_back(now_port);
        }
        sort(other_ip.begin(), other_ip.end());
        this->other_ip = other_ip;
        other_pc_num = other_ip.size();
        rpcServer = new server(host_port, dataBase);
        rpcServer->run();
        for(int i = 0; i < other_pc_num; i++){
            rpcClient[i] = new client(other_ip[i] + ":" + to_string(other_port[i]));
        }
        for(int i = 0; i < other_pc_num; i++){
            if(rpcClient[i]->init() == 0){
                cout << host_ip << " init fail" << endl;
                sleep(1);
            }
        }
        cout << "test connect" << endl;
        for(int i = 0; i < other_pc_num; i++){
            while(1){
                auto connect = rpcClient[i]->call(-1);
                if(connect== 1 || connect == 0){
                    cout << host_ip << ' '  << "connect" << other_ip[i] << "success" << endl;
                    break;
                }
                else {
                    cout << host_ip << ' '  << "connect" << other_ip[i] << "fail" << endl;
                    sleep(1);
                }
            }
        }
        rpcServer->setState(1);
        cout << "test ok" << endl;
        for(int i = 0; i < other_pc_num; i++){
            while(1){
                auto connect = rpcClient[i]->call(-1);
                if(connect == 1){
                    cout << "waiting connect success" << other_ip[i] << endl;
                    ok[i] = 1;
                    break;
                }
                else {
                    cout << "waiting connect fail" << other_ip[i]<< endl;
                    sleep(1);
                }
            }
        }
        for(int i = 0; i < other_pc_num; i++)myTaskQueue[i] = new taskQueue(rpcClient[i]);
        cout << "Open database complete. time spent is " << (now() - start).count() << endl;
        sleep(2);
    }

    void write(const User * user) {
        dataBase->write(user);
    }
    int Read(int32_t select_column, int32_t where_column, const void *column_key, size_t column_key_len, void *res){
        if(where_column == Id || where_column == Userid){
            int len = dataBase->read(select_column, where_column, column_key, column_key_len, res);
            if(len != 0)return 1;
            string response;
            int rpcLen = 0;
            vector<int>now;
            promise<pair<int, string>>myFuture[3];
            for(int i = 0; i < other_pc_num; i++){
                if(!ok[i])continue;
                now.push_back(i);
                myTaskQueue[i]->asyncCall(select_column, where_column, (char*)column_key, column_key_len, &(myFuture[i]));
            }
            for(auto i : now){
                auto nowRes = myFuture[i].get_future().get();
                //myTaskQueue[i]->profit++;
                if(nowRes.first == -1)ok[i] = 0;
                else rpcLen += nowRes.first, response += nowRes.second;
            }
            memcpy(res, response.c_str(), rpcLen * ((select_column == 0 || select_column == 3) ? 8 : 128));
            return rpcLen;
        }
        else {
            int len = dataBase->read(select_column, where_column, column_key, column_key_len, res);
            string response;
            int rpcLen = 0;
            promise<pair<int, string>> myFuture[3];
            vector<int>now;
            for(int i = 0; i < other_pc_num; i++){
                if(!ok[i])continue;
                now.push_back(i);
                myTaskQueue[i]->asyncCall(select_column, where_column, (char*)column_key, column_key_len, &(myFuture[i]));
            }
            for(auto i : now){
//                use future
                auto nowRes = myFuture[i].get_future().get();
                //myTaskQueue[i]->profit++;
                if(nowRes.first == -1)ok[i] = 0;
                else rpcLen += nowRes.first, response += nowRes.second;
            }
            if(select_column == Id || select_column == Salary){
                memcpy((char*)res + len * 8, response.c_str(), rpcLen * 8);
            }
            else {
                memcpy((char*)res + len * 128, response.c_str(), rpcLen * 128);
            }
            return len + rpcLen;
        }
    }
    ~PEngine(){
        rpcServer->setState(0);
        cout << "begin close" << endl;
        for(int i = 0; i < other_pc_num; i++){
            if(ok[i]){
                while(1){
                    cout << "begin test close " << other_ip[i] << endl;
                    auto connect = rpcClient[i]->call(-1);
                    if(connect == 1){
                        cout << "waiting close fail" << other_ip[i] << endl;
                        sleep(1);
                    }
                    else if(connect == -1){
                        ok[i] = 0;
                        break;
                    }
                    else {
                        cout << "waiting close success" << other_ip[i] << endl;
                        break;
                    }
                }
            }
        }
        for(int i = 0; i < other_pc_num; i++){
            delete myTaskQueue[i];
        }
        rpcServer->Quit();
    }
};
