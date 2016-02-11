#include "DataBase.h"
#include "Const.h"
#include "Log.h"

DataBase::DataBase():
    dataSpace()
{
    logNotice("create database");
}

/*
 *brief: 根据键名来获取值, 不存在返回 nullptr, 存在返回值
 *
 */
ValueObj *DataBase::getValue(const string &key) {
    auto dataIter = dataSpace.find(key);
    if (dataIter == dataSpace.end()) {
        return nullptr;
    }
    return dataSpace[key];
}


/*
 *brief: 根据键名来获取值
 *           不存在出错, 存在删除
 *
 */
int DataBase::deleteKey(const string &key) {
    auto dataIter = dataSpace.find(key);
    if (dataIter == dataSpace.end()) {
        return CABINET_ERR;
    }
    dataSpace.erase(key);
    return CABINET_OK;
}

int DataBase::insertKey(const string &key, ValueObj *value) {
    auto dataIter = dataSpace.find(key);
    if (dataIter != dataSpace.end()) {
        return CABINET_ERR;
    }
    dataSpace[key] = value;
    return CABINET_OK;
}
