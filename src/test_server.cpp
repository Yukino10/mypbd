//
// Created by yuki on 22-7-27.
//

#include <iostream>
#include <string.h>
#include <thread>
#include <mutex>
#include "vector"
#include "unordered_map"
#include "set"
#include "interface.h"
#include <chrono>
#include <map>
#include "bits/stdc++.h"

using namespace std;

using namespace std :: chrono;
class TestUser
{
public:
    int64_t id;
    char user_id[128];
    char name[128];
    int64_t salary;
};

milliseconds Now() {
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}
enum TestColumn{Id=0,Userid,Name,Salary};


const int MAX = 1e6 + 5;
thread tt[100];
TestUser user[MAX];
unordered_map<int64_t, int>vis1;
unordered_map<string, int>vis2;
vector<int>salary_index[MAX];
int64_t get_id(){
    int64_t now;
    while(1){
        now = 1ll * rand() * rand();
        if(rand() % 2 == 0)now *= -1;
        if(vis1.find(now) == vis1.end()){
            vis1[now] = 1;
            return now;
        }
    }
    return -1;
}
string get_uid(){
    string now;
    while(1){
        for(int i = 0; i < 128; i++)now += rand() % 26 + 'a';
        if(vis2.find(now) == vis2.end()){
            vis2[now] = 1;
            return now;
        }
    }
}
string get_name(){
    string now;
    for(int i = 0; i < 3; i++)now += rand() % 26 + 'a';
    for(int i = 3; i < 128; i++)now += 'a';
    return now;
}
int64_t get_salary(){
    return rand() % 102400;

}


void write(int x, int y, void *ctx){
    for(int i = x; i <= y; i++){
        //user[i].salary = x / 20000;
        engine_write(ctx, &user[i], sizeof(user[i]));
    }
}

mutex mut[10];
void puts(char *s){
    for(int i = 0; i < 128; i++)cout << s[i];
}
int error;
void print(void *value, void * res, int i, int j, int x, int true_num, int find_num){
    mut[0].lock();
    error++;
    cout << j << ' ' << i << ' ' << x <<  '\n';
    cout << "true" << ' ';
    if(i == 0 || i == 1){
        if(j == 0 || j == 3){
            cout << (*(int64_t*)(value)) << '\n';
            cout << "find" << ' ';
            cout << (*(int64_t*)(res)) << '\n';
        }
        else {
            puts((char*)value);
            cout << '\n';
            cout << "find" << ' ';
            puts((char*)res);
            cout << '\n';
        }
    }
    else {
        cout << true_num << '\n' << "find " << find_num << '\n';
    }
    mut[0].unlock();
}
char re[50][128 * 20000];
void read(void *ctx, int num, int limit, char* res){
    while(num--){
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 4; j++){
                //if(!(i == 0 && j == 3))continue;

                if(i == j)continue;
                if(i == 2)continue;
                //if(!(i == 0 && j == 2))continue;
                int x = rand() % limit, len = 0;
                void *key, *value;
                int key_len, value_len;

                if(i == 0)key = &user[x].id, key_len = 8;
                else if(i == 1)key = user[x].user_id, key_len = 128;
                else if(i == 2)key = user[x].name, key_len = 128;
                else key = &user[x].salary, key_len = 8;

                if(j == 0)value = &user[x].id, value_len = 8;
                else if(j == 1)value = user[x].user_id, value_len = 128;
                else if(j == 2)value = user[x].name, value_len = 128;
                else value = &user[x].salary, value_len = 8;
                len = engine_read(ctx, j, i, key, key_len, res);
                continue;
                if(i == 1 || i == 0){
                    if(memcmp(value, res, value_len) != 0){
                        print(value, res, i, j, x, 0, 0);
                    }
                    else {
                        //print(value, res, i, j, x, 0, 0);
                    }
                }
                else {
                    int64_t Key = *((int64_t*)(key));
                    if(j == 0){
                        multiset<int64_t>se_true, se_find;
                        for(auto i : salary_index[Key]){
                            se_true.insert(user[i].id);
                        }
                        for(int i = 0; i < len; i++){
                            se_find.insert(*((int64_t*)(res + 8 * i)));
                        }
                        if(se_true != se_find){
                            print(value, res, i, j, x, se_true.size(), se_find.size());
                        }
                    }
                    else if(j == 1 || j == 2){
                        multiset<string>se_true, se_find;
                        for(auto i : salary_index[Key]){
                            string now;
                            if(j == 1){
                                for(int p = 0; p < 128; p++)now += user[i].user_id[p];
                            }
                            else {
                                for(int p = 0; p < 128; p++)now += user[i].name[p];
                            }
                            se_true.insert(now);
                        }
                        for(int i = 0; i < len; i++){
                            string now;
                            for(int p = 0; p < 128; p++){
                                now += *(res + p + i * 128);
                            }
                            se_find.insert(now);
                        }
                        if(se_true != se_find){
                            print(value, res, i, j, x, se_true.size(), se_find.size());
                        }
                    }
                }
            }
        }
    }
}

char *peer_host_info[] = {"127.0.0.1:3000"};
int main() {
    int numThread = 50;
    int numUser = 10000;
    int tot = 0;
    for (int i = 0; i < numUser; i++) {
        user[i].id = get_id();
        string s = get_uid();
        for (int j = 0; j < 128; j++) {
            user[i].user_id[j] = s[j];
        }
        s = get_name();
        for (int j = 0; j < 128; j++) {
            user[i].name[j] = s[j];
        }
        user[i].salary = get_salary();
        salary_index[user[i].salary].push_back(i);
    }
    void *ctx = engine_init("127.0.0.1:3001", peer_host_info, 1, "/home/yuki/pmemdir/", "/home/yuki/test2/");
    int num = numUser / numThread;

    cout << "writing data!" << endl;
    milliseconds start = Now();

//    for (int i = 0; i < numThread; i++) {
//        tt[i] = thread(&write, i * num, i * num + num - 1, ctx);
//    }
//    for (int i = 0; i < numThread; i++) {
//        tt[i].join();
//    }
    cout << "write data complete. time spent is " << (Now() - start).count() << "ms" << endl;
    cout << "write num : " << numUser << endl;
   // engine_deinit(ctx);
//    ctx = engine_init("127.0.0.1:3001", peer_host_info, 1, "/home/yuki/pmemdir/", "/home/yuki/test2/");
    start = Now();
    cout << "reading data!" << endl;
//    for (int i = 0; i < numThread; i++) {
//        tt[i] = thread(&read, ctx, num, num * numThread, re[i]);
//    }
//    for (int i = 0; i < numThread; i++) {
//        tt[i].join();
//    }

    cout << "read data complete. time spent is " << (Now() - start).count() << "ms" << endl;
    cout << "read num : " << num * 9 * numThread << " correct num : " << num * 9 * numThread - error << endl;
    engine_deinit(ctx);
    //    void* ctx = engine_init(nullptr, nullptr, 0, "/home/yuki/CLionProjects/pbd/src/aep", "/mnt/disk/");
//    engine_write(ctx, &user[0], sizeof(user[0]));
//    char res[128];
//    engine_read(ctx, 2, 0, &user[0], 8, res);
//    for(int i = 0; i < 128; i++)cout << res[i]; cout << endl;
//    for(int i = 0 ; i < 128; i++)cout << user[0].name[i];
}
