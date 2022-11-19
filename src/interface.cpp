#pragma once
#include "interface.h"
#include <iostream>
#include <vector>
#include <cstring>
#include "Engine.cpp"



//std::vector<User> users;
//
//void engine_write( void *ctx, const void *data, size_t len) {
//    User user;
//    memcpy(&user,data,len);
//    users.push_back(user);
// }
//
//size_t engine_read( void *ctx, int32_t select_column,
//    int32_t where_column, const void *column_key, size_t column_key_len, void *res) {
//    int users_size = users.size();
//    bool b = true;
//    size_t res_num = 0;
//    for(int i=0;i<users_size;++i)
//    {
//        switch(where_column) {
//            case Id: b = memcmp(column_key,&users[i].id,column_key_len) == 0; break;
//            case Userid: b = memcmp(column_key,users[i].user_id,column_key_len) == 0; break;
//            case Name: b = memcmp(column_key,users[i].name,column_key_len) == 0; break;
//            case Salary: b = memcmp(column_key,&users[i].salary,column_key_len) == 0; break;
//            default: b = false; break; // wrong
//        }
//        if(b)
//        {
//            ++res_num;
//            switch(select_column) {
//                case Id:
//                    memcpy(res, &users[i].id, 8);
//                    res = (char *)res + 8;
//                    break;
//                case Userid:
//                    memcpy(res, users[i].user_id, 128);
//                    res = (char *)res + 128;
//                    break;
//                case Name:
//                    memcpy(res, users[i].name, 128);
//                    res = (char *)res + 128;
//                    break;
//                case Salary:
//                    memcpy(res, &users[i].salary, 8);
//                    res = (char *)res + 8;
//                    break;
//                default: break; // wrong
//            }
//        }
//    }
//    return res_num;
//}
//
//void* engine_init(const char* host_info, const char* const* peer_host_info, size_t peer_host_info_num,
//                  const char* aep_dir, const char* disk_dir) {return nullptr;}
//
//void engine_deinit(void *ctx) {}

/*
 * Initialization interface, which is called when the engine starts.
 * You need to create or recover db from pmem-file.
 * host_info: Local machine information including ip and port. This value is nullptr in the preliminary round.
 * peer_host_info: Information about other machines in the distributed cluster. This value is nullptr in the preliminary round.
 * peer_host_info_num: The num of other machines in the distributed cluster.  This value is 0 in the preliminary round.
 * aep_dir: AEP file directory, eg : "/mnt/aep/"
 * disk_dir: Disk file directory, eg : "/mnt/disk/"
 */
void* engine_init(const char* host_info, const char* const* peer_host_info, size_t peer_host_info_num,
                  const char* aep_dir, const char* disk_dir) {
    string s, ss, host_ip;
    int host_port = 0;
    vector<string> other_ip;
    vector<int> other_port;
    for(int i = 0; i < strlen(aep_dir); i++)s += aep_dir[i];
    for(int i = 0; i < strlen(disk_dir); i++)ss += disk_dir[i];
    int flag = 0;
    for(int i = 0; i < strlen(host_info); i++){
        if(flag)host_port *= 10, host_port += host_info[i] - '0';
        if(host_info[i] != ':' && !flag)host_ip += host_info[i];
        if(host_info[i] == ':')flag = 1;
    }
    for(int i = 0; i < peer_host_info_num; i++){
        string now;
        int flag = 0, port = 0;
        for(int j = 0; j < strlen(peer_host_info[i]); j++){
            if(flag)port *= 10, port += peer_host_info[i][j] - '0';
            if(peer_host_info[i][j] == ':')other_ip.push_back(now), flag = 1;
            else now += peer_host_info[i][j];
        }
        other_port.push_back(port);
    }
    Engine * engine = new Engine(s, ss, host_ip, host_port, other_ip, other_port);
    return engine;
}
void engine_write( void *ctx, const void *data, size_t len) {
    User user;
    memcpy(&user,data,len);
    Engine * engine = (Engine*)ctx;
    engine->write(&user);
 }

size_t engine_read( void *ctx, int32_t select_column,
    int32_t where_column, const void *column_key, size_t column_key_len, void *res) {
    Engine * engine = (Engine*)ctx;
    return engine->read(select_column, where_column, column_key, column_key_len, res);
}

void engine_deinit(void *ctx) {
    Engine * engine = (Engine*)ctx;
    delete engine;
}


