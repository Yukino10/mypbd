

//
// Created by yuki on 22-7-25.
//
#include <stdint.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mutex>
#include "hash.cpp"
#include "User.cpp"
#include "myHash.cpp"
#include <errno.h>
#include <malloc.h>
#include "libpmem.h"
#include <libpmemobj.h>
using namespace std;

class File {
private:
    char * base = nullptr;
    PMEMobjpool *pop = nullptr;
    atomic_uint32_t nowPos;
    int is_pmem;

public:
    File(const std::string &path, const int &id, const bool &exist)
    {
        this->is_pmem = is_pmem;
        stringstream s;
        s << path  << id;
        char filename[30];
        memset(filename, 0, sizeof(filename));
        for(int i = 0; i < s.str().size(); i++)filename[i] = s.str()[i];
        //filename[s.str().size()] = 'a';
        pop = pmemobj_create(filename, filename,TRUE_PER_FILE_SIZE , 0666);
        if (pop != nullptr) {
            if(id == 0)cout << id << ' ' << "pmemobj_create successfully" << endl;
        } else {
            pop = pmemobj_open(filename, filename);
            if (pop == nullptr) {
                if(id == 0)cout << id << ' ' << "failed to open the pool" << endl;
                return;
            }
        }
        PMEMoid root = pmemobj_root(pop, PER_FILE_SIZE);
        base = (char *)pmemobj_direct(root);
        if(base == nullptr) {
            cout << id << ' ' << "rootP is NULL, please check your persistent memory pool" << endl;
            return;
        }
        nowPos = PER_FILE_NUM - 1;
        //memset(base, 0, PER_FILE_NUM);
        getNoDataPos();
        //forcePut();
    }

//    void put(const User * user) {
//        u_int64_t pos = 0;
//        char s[PER_CPU_CACHE_SIZE];
//        mu1.lock();
//        memcpy(base + *(u_int64_t*)(base + 4) * 272 + 8, user, 272);
//        if(*(u_int64_t*)(base + 4) == PER_CPU_CACHE_NUM - 1){
//            pos = (*(u_int64_t*)base) * 272;
//            pos = pos % (PER_FILE_SIZE - PER_CPU_CACHE_SIZE - 8);
//            memcpy(s, base + 8, PER_CPU_CACHE_SIZE);
//            *(u_int64_t*)base = *(u_int64_t*)base + PER_CPU_CACHE_NUM;
//        }
//        (*(u_int64_t*)(base + 4))++;
//        mu1.unlock();
//        if(pos){
//            memcpy(base + pos + 8 + PER_CPU_CACHE_SIZE, s, PER_CPU_CACHE_SIZE);
//            pmemobj_persist(pop, base + pos + 8 + PER_CPU_CACHE_SIZE, PER_CPU_CACHE_SIZE);
//        }
//    }
//    void put(const User * user) {
//        memcpy(base + *(u_int64_t*)(base + 4) * 272 + 8, user, 272);
//        if(*(u_int64_t*)(base + 4) == PER_CPU_CACHE_NUM - 1){
//            memcpy(base + (*(u_int64_t*)base) * 272 + 8 + PER_CPU_CACHE_SIZE, base + 8, PER_CPU_CACHE_SIZE);
//            pmemobj_persist(pop, base + (*(u_int64_t*)base) * 272 + 8 + PER_CPU_CACHE_SIZE, PER_CPU_CACHE_SIZE);
//            //*(u_int64_t*)base = (*(u_int64_t*)base + PER_CPU_CACHE_NUM) % (PER_FILE_NUM - 20000);
//            *(u_int64_t*)base = (*(u_int64_t*)base + PER_CPU_CACHE_NUM) % (PER_FILE_NUM - PER_CPU_CACHE_NUM - 8);
//        }
//        else (*(u_int64_t*)(base + 4))++;
//    }
    void put(const User * user, u_int64_t pos){
        //pos %=  (MAX_NUM / FILE_NUM);
        memcpy(base + pos * 272 + PER_FILE_NUM , user, 272);
        pmemobj_persist(pop, base + pos * 272 + PER_FILE_NUM, 272);
    }

    void signPos(u_int64_t pos){
        *((u_int8_t *)base + pos) = 1;
    }
//    void forcePut(){
//        if(*(u_int64_t*)(base + 4) != 0){
//            memcpy(base + (*(u_int64_t*)base) * 272 + 8 + PER_CPU_CACHE_SIZE, base + 8, (*(u_int64_t*)(base + 4)) * 272);
//            pmemobj_persist(pop, base + (*(u_int64_t*)base) * 272 + 8 + PER_CPU_CACHE_SIZE, (*(u_int64_t*)(base + 4)) * 272 );
//            *(u_int64_t*)base = *(u_int64_t*)base + *(u_int64_t*)(base + 4);
//        }
//    }
    void*  getDataBase(){
        return base + PER_FILE_NUM;
    }

    u_int64_t getMaxDataLen(){
        return PER_FILE_NUM;
    }

    int isDataHere(u_int32_t pos){
        return *((u_int8_t *)base + pos);
    }

    void getNoDataPos(){
        //if(nowPos > PER_FILE_NUM / 2)memset(base, 0, PER_FILE_NUM / 8), nowPos = 0;
        while(nowPos && !isDataHere(nowPos))nowPos--;
    }

    u_int32_t getNowDataPos(){
        return ++nowPos;
    }
    ~File() {
        pmemobj_close(pop);
    }
};

