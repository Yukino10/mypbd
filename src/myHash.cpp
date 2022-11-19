//
// Created by yuki on 22-8-15.
//
#include <iostream>
#include "params.h"
class Node{
public:
   u_int32_t value, nextPos;
   u_int64_t key;
};

//superHash
class myHash{
public:
    u_int32_t* head;
    u_int32_t* lastPos;
    Node* data;
    u_int64_t maxSize;
    atomic<u_int32_t> nowDataPos;
public:
    myHash(u_int64_t Size){
        data = new Node [Size];
        for(int i = 0; i < Size; i++){
            data[i].nextPos = 0;
        }
        maxSize = Size * HASH_FACTOR;
        head = new u_int32_t [maxSize];
        lastPos = new u_int32_t [maxSize];
        for(int i = 0; i < maxSize; i++){
            head[i] = 0;
            lastPos[i] = 0;
        }
        nowDataPos = 0;
    }
    //One in 100 million probability of a bug
    void insert(u_int64_t key, u_int32_t value){
        u_int32_t pos = key % maxSize;
        u_int32_t truePos = ++nowDataPos;
        data[truePos].key = key, data[truePos].value = value;
        if(head[pos] == 0){
            head[pos] = truePos;
            lastPos[pos] = truePos;
        }
        else {
            data[lastPos[pos]].nextPos = truePos;
            lastPos[pos] = truePos;
        }
    }
    u_int32_t* find(u_int64_t key){
        u_int32_t pos = key % maxSize;
        if(head[pos] == 0){
            return nullptr;
        }
        else {
            u_int32_t nowPos = head[pos];
            while(nowPos != 0){
                if(data[nowPos].key == key){
                    return &(data[nowPos].value);
                }
                nowPos = data[nowPos].nextPos;
            }
            return nullptr;
        }
    }
    ~myHash(){
        delete []head;
        delete []lastPos;
        delete []data;
    }
};

//开放地址法
//class myHash{
//public:
//    Node** head;
//    u_int64_t maxSize;
//public:
//    myHash(u_int64_t Size){
//        maxSize = Size * HASH_FACTOR;
//        head = new Node* [maxSize];
//        for(int i = 0; i < maxSize; i++){
//            head[i] = nullptr;
//        }
//    }
//    void insert(u_int64_t key, u_int64_t value){
//        u_int64_t pos = key % maxSize;
//        while(!(head[pos] == nullptr || head[pos]->key == key)){
//            pos = (pos + 1) % maxSize;
//        }
//        if(head[pos] == nullptr){
//            head[pos] = new Node(key, value);
//        }
//        else {
//            head[pos]->set(key, value);
//        }
//    }
//    u_int64_t* find(u_int64_t key){
//        u_int64_t pos = key % maxSize;
//        while(!(head[pos] == nullptr || head[pos]->key == key)){
//            pos = (pos + 1) % maxSize;
//        }
//        if(head[pos] == nullptr){
//            return nullptr;
//        }
//        else {
//            return &(head[pos]->value);
//        }
//    }
//    ~myHash(){
//        for(int i = 0; i < maxSize; i++){
//            if(head[i] != nullptr){
//                delete head[i];
//            }
//        }
//        delete head;
//    }
//};
//superMultiHash
class myMultiHash{
public:
    u_int32_t* head;
    u_int32_t* lastPos;
    Node* data;
    u_int64_t maxSize;
    atomic<u_int32_t> nowDataPos;
    PMutex mu;
public:
    myMultiHash(u_int64_t Size){
        data = new Node [Size];
        for(int i = 0; i < Size; i++){
            data[i].nextPos = 0;
        }
        maxSize = Size * HASH_FACTOR;
        head = new u_int32_t [maxSize];
        lastPos = new u_int32_t [maxSize];
        for(int i = 0; i < maxSize; i++){
            head[i] = 0;
            lastPos[i] = 0;
        }
        nowDataPos = 0;
    }
    void insert(u_int64_t key, u_int32_t value){
        u_int32_t pos = key % maxSize;
        u_int32_t truePos = ++nowDataPos;
        data[truePos].key = key, data[truePos].value = value;
        mu.lock();
        while(!(head[pos] == 0 || data[head[pos]].key == key))pos = (pos + 1) % maxSize;
        if(head[pos] == 0){
            head[pos] = truePos;
            lastPos[pos] = truePos;
        }
        else {
            data[lastPos[pos]].nextPos = truePos;
            lastPos[pos] = truePos;
        }
        mu.unlock();
    }
    u_int32_t find(u_int64_t key){
        u_int64_t pos = key % maxSize;
        while(!(head[pos] == 0 || data[head[pos]].key == key))pos = (pos + 1) % maxSize;
        return head[pos];
    }
    ~myMultiHash(){
        delete []head;
        delete []lastPos;
        delete []data;
    }
};
//mymultihash，开放地址+链地址
//class myMultiHash{
//public:
//    Node** head, **foot;
//    u_int64_t maxSize;
//public:
//    myMultiHash(u_int64_t Size){
//        maxSize = Size * HASH_FACTOR;
//        head = new Node* [maxSize];
//        foot = new Node* [maxSize];
//        for(int i = 0; i < maxSize; i++){
//            head[i] = nullptr;
//            foot[i] = nullptr;
//        }
//    }
//    void insert(u_int64_t key, u_int64_t value){
//        u_int64_t pos = key % maxSize;
//        while(!(head[pos] == nullptr || head[pos]->key == key)){
//            pos = (pos + 1) % maxSize;
//        }
//        if(head[pos] == nullptr){
//            head[pos] = new Node(key, value);
//            foot[pos] = head[pos];
//        }
//        else {
//            foot[pos] = foot[pos]->insert(key, value);
//        }
//    }
//    Node* find(u_int64_t key){
//        u_int64_t pos = key % maxSize;
//        while(!(head[pos] == nullptr || head[pos]->key == key)){
//            pos = (pos + 1) % maxSize;
//        }
//        return head[pos];
//    }
//    ~myMultiHash(){
//        for(int i = 0; i < maxSize; i++){
//            if(head[i] != nullptr){
//                delete head[i];
//            }
//        }
//        delete head;
//        delete foot;
//    }
//};

//mulitihash，链地址法，find额外处理
//class myMultiHash{
//public:
//    Node** head, **foot;
//    u_int64_t maxSize;
//public:
//    myMultiHash(u_int64_t Size){
//        maxSize = Size * HASH_FACTOR;
//        head = new Node* [maxSize];
//        foot = new Node* [maxSize];
//        for(int i = 0; i < maxSize; i++){
//            head[i] = nullptr;
//            foot[i] = nullptr;
//        }
//    }
//    void insert(u_int64_t key, u_int64_t value){
//        u_int64_t pos = key % maxSize;
//        if(head[pos] == nullptr){
//            head[pos] = new Node(key, value);
//            foot[pos] = head[pos];
//        }
//        else {
//            foot[pos] = foot[pos]->insert(key, value);
//        }
//    }
//    Node* find(u_int64_t key){
//        return head[pos];
//    }
//    ~myMultiHash(){
//        for(int i = 0; i < maxSize; i++){
//            if(head[i] != nullptr){
//                delete head[i];
//            }
//        }
//        delete head;
//        delete foot;
//    }
//};



//const int FILE_NUM = 16;
////unordered_map<u_int64_t, u_int64_t>ma;
//unordered_multimap<u_int64_t, u_int64_t>ma;
//const int n = 10000000;
//int key[n], value[n];

//int main(){
//    ios::sync_with_stdio(0);
//    cin.tie(0);cout.tie(0);
//    //freopen("in.txt","r",stdin);
//    //int t;cin>>t;
//    //while(t--)solve(),cout<<'\n'
//    for(int i = 0;  i < n; i++){
//        key[i] = rand();
//        value[i] = rand();
//
//    }
//    milliseconds start = now();
//    for(int i = 0; i < n; i++){
//        ma.insert({key[i], value[i]});
//    }
//    cout << (now() - start).count() << endl;
//    start = now();
//    for(int i = 0; i < n; i++){
//        auto it = ma.find(key[i]);
//    }
//    cout << (now() - start).count() << endl;
//
//    myMultiHash *ha = new myMultiHash(n);
//    start = now();
//    for(int i = 0; i < n; i++){
//        ha->insert(key[i], value[i]);
//    }
//    cout << (now() - start).count() << endl;
//    start = now();
//    for(int i = 0; i < n; i++){
//        Node* data = ha->find(key[i]);
//    }
//    cout << (now() - start).count() << endl;
//    int flag = 1;
//    for(int i = 0; i < n / 100; i++){
//        set<int>se, see;
//        auto it = ma.find(key[i]);
//        int len = ma.count(key[i]);
//        for(int j = 0; j < len; j++){
//            se.insert((*it).second);it++;
//        }
//        Node* data = ha->find(key[i]);
//        while(data != nullptr){
//            see.insert(data->value);
//            data = data->next;
//        }
//        if(se != see){
//            flag = 0;
//            break;
//        }
//    }
//    cout << flag << endl;
//    delete ha;
//    while(1){
//        int x = 0;
//    }
//
//}
//10000000000002137
//12134793327482137
//18446744073709551615
//15896579258970260209

