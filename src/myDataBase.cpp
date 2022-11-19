//
// Created by yuki on 22-10-2.
//

#include <sys/mman.h>
#include <stdlib.h>
#include <string>
#include <atomic>
#include "File.cpp"
#include "list"
using namespace std;
class myDataBase {
public:
    File *Files[FILE_NUM];
    User *myData[FILE_NUM];
    myHash *id;
    myHash *user_id;
    myMultiHash *salary[FILE_NUM];
    int64_t idMax, idMin, salaryMax, salaryMin;
    char userIdMax[128], userIdMin[128];
    PMutex doorMu;
public:
    myDataBase(string &ape_dir, string &ssd_dir){
        id = new myHash(MAX_NUM);
        user_id = new myHash(MAX_NUM);
        for(u_int64_t i = 0; i < FILE_NUM; i++){
            salary[i] = new myMultiHash(MAX_NUM / FILE_NUM);
        }
        for(int fileId = 0; fileId < FILE_NUM; fileId++) {
            Files[fileId] = new File(ape_dir, fileId,  1);
            myData[fileId] = (User*)Files[fileId]->getDataBase();
        }
        idMax = -9223372036854775808 , idMin = 9223372036854775807;
        salaryMax = -9223372036854775808 , salaryMin = 9223372036854775807 ;
        memset(userIdMin, -1, 128);
        memset(userIdMax, 0, 128);
        u_int64_t Id, Salary, h_user_id;
        for(u_int64_t i = 0; i < FILE_NUM; i++){
            for(u_int64_t j = 0; j < Files[i]->getMaxDataLen(); j++){
                if(!Files[i]->isDataHere(j))continue;
                Id = myData[i][j].id;
                h_user_id = get_hash(myData[i][j].user_id);
                Salary = myData[i][j].salary;
                u_int32_t pos = j | (i << (32 - FILE_BIT_NUM));
                id->insert(Id, pos);
                user_id->insert(h_user_id, pos);
                salary[i]->insert(Salary, pos);
                idMax = max(idMax, (int64_t)Id), idMin = min(idMin, (int64_t)Id);
                if(memcmp(myData[i][j].user_id, userIdMax, 128) > 0)memcpy(userIdMax, myData[i][j].user_id, 128);
                if(memcmp(myData[i][j].user_id, userIdMin, 128) < 0)memcpy(userIdMin, myData[i][j].user_id, 128);
                salaryMax = max(salaryMax, (int64_t)Salary), salaryMin = min(salaryMin, (int64_t)Salary);
            }
        }
    }
    u_int64_t getFileId(int64_t Salary){
        return (Salary % FILE_NUM + FILE_NUM) % FILE_NUM;
    }
    void write(const User * user) {
        doorMu.lock();
        idMax = max(idMax, user->id), idMin = min(idMin, user->id);
        salaryMax = max(salaryMax, user->salary), salaryMin = min(salaryMin, user->salary);
        doorMu.unlock();
        u_int64_t fileId = getFileId(user->salary);
        u_int32_t nextPos = Files[fileId]->getNowDataPos();
        Files[fileId]->put(user, nextPos);
        u_int32_t combinationPos = nextPos | (fileId << (32 - FILE_BIT_NUM));
        id->insert(user->id, combinationPos);
        user_id->insert(get_hash(user->user_id), combinationPos);
        salary[fileId]->insert(user->salary, combinationPos);
        Files[fileId]->signPos(nextPos);
    }
    int read(int32_t select_column, int32_t where_column, const void *column_key, size_t column_key_len, void *res){
        if(where_column == Id){
            if(*((int64_t*)column_key) > idMax || *((int64_t*)column_key) < idMin)return 0;
            u_int32_t *it = id->find(*(u_int64_t *)column_key);
            if(it == nullptr)return 0;
            u_int32_t pos = *it;
            u_int32_t fileId = pos >> (32 - FILE_BIT_NUM), filePos = (pos << FILE_BIT_NUM) >> FILE_BIT_NUM;
            if(select_column == Id){
                memcpy(res, column_key, 8);
            }
            else if(select_column == Userid){
                memcpy(res, myData[fileId][filePos].user_id, 128);
            }
            else if(select_column == Name){
                memcpy(res, myData[fileId][filePos].name, 128);
            }
            else {
                memcpy(res, &myData[fileId][filePos].salary, 8);
            }
            return 1;
        }
        else if(where_column == Userid){
            u_int64_t ha = get_hash((char*)column_key);
            u_int32_t *it = user_id->find(ha);
            if(it == nullptr)return 0;
            u_int32_t pos = *it;
            u_int32_t fileId = pos >> (32 - FILE_BIT_NUM), filePos = (pos << FILE_BIT_NUM) >> FILE_BIT_NUM;
            if(select_column == Id){
                memcpy(res, &myData[fileId][filePos].id, 8);
            }
            else if(select_column == Userid){
                memcpy(res, column_key, 128);
            }
            else if(select_column == Name){
                memcpy(res, myData[fileId][filePos].name, 128);
            }
            else {
                memcpy(res, &myData[fileId][filePos].salary, 8);
            }
            return 1;
        }
        else if(where_column == Salary){
            if(*((int64_t*)column_key) > salaryMax || *((int64_t*)column_key) < salaryMin)return 0;
            u_int64_t key = *((u_int64_t*)column_key);
            u_int32_t fileId = getFileId(key);
            u_int32_t it = salary[fileId]->find(key);
            if(it == 0)return 0;
            u_int32_t len = 0;
            while(it != 0){
                u_int32_t pos = (salary[fileId]->data[it]).value;
                u_int32_t filePos = (pos << FILE_BIT_NUM) >> FILE_BIT_NUM;
                if(select_column == Id){
                    memcpy(res, &myData[fileId][filePos].id, 8), res = (char*)res + 8;
                }
                else if(select_column == Userid){
                    memcpy(res, myData[fileId][filePos].user_id, 128), res = (char*)res + 128;
                }
                else if(select_column == Name){
                    memcpy(res, myData[fileId][filePos].name, 128), res = (char*)res + 128;
                }
                else{
                    memcpy(res, &myData[fileId][filePos].salary, 8), res = (char*)res + 8;
                }
                it = (salary[fileId]->data[it]).nextPos;
                len++;
            }
            return  len;
        }
        return 0;
    }
    ~myDataBase(){
        for(int i = 0; i < FILE_NUM; i++){
            delete Files[i];
            delete salary[i];
        }
        delete id;
        delete user_id;
    }
};