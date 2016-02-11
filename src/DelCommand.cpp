#include "DelCommand.h"
#include "Log.h"
#include "DataBase.h"
#include "StringObj.h"

#ifdef CABINET_SERVER
/*
 *brief: 首先检查用户出入解析结果是否正确,
 *      检查数据空间中, 是否有对应的键, 没有返回错误
 *      shanchu值, 返回
 *
 */
int DelCommand::operator()(Client *client) const {
    //check user resolved input
    const vector<string> &argv = client->getReceiveArgv();
    if (argv.size() != (unsigned int)(this->commandArgc() + 1)) {
        logWarning("delete command receive error number of argv! argv_len[%d]", argv.size());
        return CABINET_ERR;
    }   

    const string &keyName = argv[1];
    //check key exist
    DataBase *db = client->getDataBase();
    ValueObj *value = nullptr;
    if ((value = db->getValue(keyName)) == nullptr) {
        logDebug("delete no-exist key, key[%s]", keyName.c_str());
        client->initReplyHead(1);
        client->appendReplyBody(string("Pardon?"));
        return CABINET_OK;
    }

    //delete key
    if (db->deleteKey(keyName) == CABINET_ERR) {
        logDebug("delete key error, key[%s]", keyName.c_str());
        client->initReplyHead(1);
        client->appendReplyBody(string("Delete Key Error"));
        return CABINET_ERR;
    }
    else {
        logDebug("delete key, key[%s]", keyName.c_str());
        client->initReplyHead(1);
        client->appendReplyBody(string("OK"));
        return CABINET_OK;
    }
}
#endif
