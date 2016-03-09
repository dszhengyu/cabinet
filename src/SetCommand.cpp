#include "SetCommand.h"
#include "Log.h"
#include "StringObj.h"
#include "ServerClient.h"

#ifdef CABINET
/*
 *brief: 首先检查用户出入解析结果是否正确,
 *       检查数据空间中, 是否有对应的键
 *              有对应的键, 如果类型正确, 修改键值
 *              如果类型错误, 返回出错
 *       无对应的键, 设置值, 返回成功
 *
 */
int SetCommand::operator()(Client *client) const {
    ServerClient *serverClient = (ServerClient *)client;
    //check user resolved input
    const vector<string> &argv = client->getReceiveArgv();
    if (argv.size() != (unsigned int)(this->commandArgc() + 1)) {
        logWarning("set command receive error number of argv! argv_len[%d]", argv.size());
        client->initReplyHead(1);
        client->appendReplyBody(string("set no complete yet"));
        return CABINET_ERR;
    }   

    const string &keyName = argv[1];
    const string &newValue = argv[2];
    //check key existence
    DataBase *db = serverClient->getDataBase();
    ValueObj *value = nullptr;
    //1. key exist
    if ((value = db->getValue(keyName)) != nullptr) {
        logDebug("set exist key, key[%s]", keyName.c_str());
        //check key type
        if (value->getValueType() != ValueObj::STRING_TYPE) {
            logDebug("set command execute in wrong type of value, key[%s] value_type[%d]", keyName.c_str(), value->getValueType());
            client->initReplyHead(1);
            client->appendReplyBody(string("I'm not string type."));
            return CABINET_OK;
        }
        logDebug("set string value, key[%s], new_value[%s]", keyName.c_str(), newValue.c_str());
        ((StringObj *)value)->set(newValue);
        client->initReplyHead(1);
        client->appendReplyBody(string("ok"));
        return CABINET_OK;
    }
    //2. key does not exist
    else {
        logDebug("set no-exist key, key[%s]", keyName.c_str());
        //to-do, insert new thing in dataspace
        ValueObj *newObj = new StringObj(newValue);
        if (db->insertKey(keyName, newObj) == CABINET_ERR) {
            logWarning("set command, insert key into dataspace error, key[%s]", keyName.c_str());
            client->initReplyHead(1);
            client->appendReplyBody(string("Set Error"));
            return CABINET_ERR;
        }
        client->initReplyHead(1);
        client->appendReplyBody(string("ok"));
        return CABINET_OK;
    }
}
#endif
