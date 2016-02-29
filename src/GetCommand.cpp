#include "GetCommand.h"
#include "Log.h"
#include "DataBase.h"
#include "StringObj.h"
#include "ServerClient.h"

#ifdef CABINET_SERVER
/*
 *brief: 首先检查用户出入解析结果是否正确,
 *      检查数据空间中, 是否有对应的键, 没有返回错误
 *      检查键对应的值的类型是否正确, 错误返回错误
 *      获取值, 返回
 *
 */
int GetCommand::operator()(Client *client) const {
    ServerClient *serverClient = (ServerClient *)client;
    //check user resolved input
    const vector<string> &argv = client->getReceiveArgv();
    if (argv.size() != (unsigned int)(this->commandArgc() + 1)) {
        logWarning("get command receive error number of argv! argv_len[%d]", argv.size());
        client->initReplyHead(1);
        client->appendReplyBody(string("get no complete yet"));
        return CABINET_ERR;
    }   

    const string &keyName = argv[1];
    //check key exist
    DataBase *db = serverClient->getDataBase();
    ValueObj *value = nullptr;
    if ((value = db->getValue(keyName)) == nullptr) {
        logDebug("get no-exist key, key[%s]", keyName.c_str());
        client->initReplyHead(1);
        client->appendReplyBody(string("Pardon?"));
        return CABINET_OK;
    }

    //check key type
    if (value->getValueType() != ValueObj::STRING_TYPE) {
        logDebug("get command execute in wrong type of value, key[%s] value_type[%d]", keyName.c_str(), value->getValueType());
        client->initReplyHead(1);
        client->appendReplyBody(string("I'm not string type."));
        return CABINET_OK;
    }

    //return value
    const string &stringValue = ((StringObj *)value)->get();
    logDebug("get correct thing, key[%s], value[%s]", keyName.c_str(), stringValue.c_str());
    client->initReplyHead(1);
    client->appendReplyBody(stringValue);
    return CABINET_OK;

}
#endif
