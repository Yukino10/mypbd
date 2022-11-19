//
// Created by yuki on 22-10-2.
//
#include "future"
#include "client.cpp"
#include "mutex"
#include "params.h"
using namespace std;

class taskQueue{
public:
    PMutex mu;
    client* rpcClient;
    thread scheduledTask;
    int _stop = 0, batchSize = 1;
    atomic_int profit;
    int lPos, rPos;
    reqData** taskData;
//    vector<int>historyProfit;
//    vector<int>historyBatchSize;
public:
    taskQueue(client* rpcClient){
        lPos = 0, rPos = 0, profit = 0;
        taskData = new reqData* [TASK_QUEUE_SIZE];
        //for(int i = 0; i < SAMPLE_NUM * 2; i++)historyProfit.push_back(0), historyBatchSize.push_back(0);
        this->rpcClient = rpcClient;
        scheduledTask = std::thread([this] {
//            while(!_stop){
//                historyProfit.push_back(profit), profit = 0;
//                historyBatchSize.push_back(batchSize);
//                int preProfit = 0, nowProfit = 0, preBatch = 0, nowBatch = 0;
//                for(int i = historyProfit.size() - 2 * SAMPLE_NUM; i < historyProfit.size() - SAMPLE_NUM; i++){
//                    preProfit += historyProfit[i];
//                    preBatch += historyBatchSize[i];
//                }
//                for(int i = historyProfit.size() - SAMPLE_NUM; i < historyProfit.size(); i++){
//                    nowProfit += historyProfit[i];
//                    nowBatch += historyBatchSize[i];
//                }
//                if(nowProfit > preProfit && nowBatch >= preBatch)batchSize++;
//                else if(nowProfit > preProfit && nowBatch <= preBatch)batchSize--;
//                else if(nowProfit < preProfit && nowBatch >= preBatch)batchSize--;
//                else if(nowProfit < preProfit && nowBatch <= preBatch)batchSize++;
//                else batchSize--;
//                batchSize = max(1, batchSize);
//                forceDoTask();
//                cout << nowProfit << ' ' << preProfit << endl;
//                cout << nowBatch << ' ' << preBatch << endl;
//                cout << "batchSize:" << batchSize << ' ' << fuck << endl;
//                std::this_thread::sleep_for(std::chrono::milliseconds(SAMPLE_INTERVAL));
//            }
            while(!_stop){
                batchSize = 8;
                if(profit < 10000)batchSize = 1;
                else if(profit < 20000)batchSize = 2;
                else if(profit < 30000)batchSize = 3;
                else if(profit < 40000)batchSize = 4;
//                else if(profit < 50000)batchSize = 5;
//                else if(profit < 60000)batchSize = 6;
//                else batchSize = 7;
                profit = 0;
                forceDoTask();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        });
    }
    void asyncCall(int select_column, int where_column, char* column_key, int column_key_len, promise<pair<int, string>>* pro){
        profit++;
        reqData* nowReq = new reqData(select_column, where_column, column_key, column_key_len, pro);
        doTask(nowReq);
    }
    void doTask(reqData *nowReq){
        mu.lock();
        taskData[rPos] = nowReq;
        rPos = (rPos + 1) % TASK_QUEUE_SIZE;
        if((rPos - lPos + TASK_QUEUE_SIZE) % TASK_QUEUE_SIZE < batchSize){
            mu.unlock();return;
        }
        u_int32_t l = lPos, r = rPos;
        lPos = r;
        mu.unlock();
        rpcClient->asyncCall(taskData, l, r);
//        while(l != r){
//            taskData[l]->promiseQ->set_value({1, "1"});
//            delete taskData[l];
//            l++;
//            l %= TASK_QUEUE_SIZE;
//        }
    }
    void forceDoTask(){
        mu.lock();
        if(lPos == rPos){
            mu.unlock();return;
        }
        u_int32_t l = lPos, r = rPos;
        lPos = r;
        mu.unlock();
        rpcClient->asyncCall(taskData, l, r);
//        while(l != r){
//            taskData[l]->promiseQ->set_value({1, "1"});
//            delete taskData[l];
//            l++;
//            l %= TASK_QUEUE_SIZE;
//        }
    }
    ~taskQueue(){
        _stop = 1;
        scheduledTask.join();
        delete []taskData;
    }
};
