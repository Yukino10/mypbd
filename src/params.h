//
// Created by yuki on 22-7-25.
//
#pragma once

#ifndef INTERFACE_PARAMS_H
#define INTERFACE_PARAMS_H

#endif //INTERFACE_PARAMS_H

#include <string>
#include <mutex>

using namespace std;
//range次数

enum Column{Id=0,Userid,Name,Salary};

const u_int64_t PAGE_SIZE = 4096;
const int FILE_NUM = 16;
const int FILE_BIT_NUM = 4;
//const u_int64_t MAX_NUM = 1000000;
const u_int64_t MAX_NUM = 200000000;
const u_int64_t MAX_SIZE = MAX_NUM * 272;

const u_int64_t TRUE_PER_FILE_SIZE = MAX_SIZE / FILE_NUM * 5 / 3;
const u_int64_t PER_CACHE_NUM = MAX_NUM / FILE_NUM / FILE_NUM * 2;
const u_int64_t PER_FILE_NUM = MAX_NUM / FILE_NUM * 5 / 4;
const u_int64_t PER_CACHE_SIZE = PER_CACHE_NUM * 272;
const u_int64_t PER_FILE_SIZE = PER_FILE_NUM * 272;
const u_int64_t PER_PC_CLIENT_NUM = 50;
const u_int64_t PER_PC_TASK_QUEUE_NUM = 4;
const u_int64_t PERSIST_THREAD_NUM = 4;
const u_int32_t TASK_QUEUE_SIZE = 10000000;
const u_int32_t SAMPLE_INTERVAL = 100; //ms
const u_int32_t SAMPLE_NUM = 1; //


const u_int64_t PER_CPU_CACHE_SIZE = (1 << 18) / FILE_NUM;//1MB/FILE_NUM

//hash
const double HASH_FACTOR = 3;
//锁
struct PMutex {
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    void lock() { pthread_mutex_lock(&m); }
    void unlock() { pthread_mutex_unlock(&m); }
};
