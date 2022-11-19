//
// Created by yuki on 22-7-14.
//

//
// Created by yuki on 22-7-14.
//
#include "string"
#include "string"
#include "iostream"
#include <sys/io.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "PEngine.cpp"

class Engine {

private:
    PEngine *pEngine;
    u_int64_t read_num = 0, write_num = 0;
    u_int64_t x[4] = {0}, y[4] = {0};
    milliseconds start = now(), startt = now();
    thread test[50];
    char res[100000];
public:
    Engine(string &ape_dir, string &ssd_dir, string &host_ip, int &host_port, vector<string>&other_ip, vector<int>&other_port){
        std::cout << "Opening database!" << std::endl;
        pEngine = new PEngine(ape_dir, ssd_dir, host_ip, host_port, other_ip, other_port);
    }
    void write(User * user){
        pEngine->write(user);
    }
    int read(int32_t select_column, int32_t where_column, const void *column_key, size_t column_key_len, void *res){
        return pEngine->Read(select_column, where_column, column_key, column_key_len, res);
    }
    ~Engine(){
        delete pEngine;
    }
};