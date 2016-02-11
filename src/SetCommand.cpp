#include "SetCommand.h"
#include "Log.h"

#ifdef CABINET_SERVER
/*
 *brief: 首先检查用户出入解析结果是否正确,
 *       检查数据空间中, 是否有对应的键
 *              有对应的键, 如果类型正确, 修改键值
 *              如果类型错误, 返回出错
 *       无对应的键, 设置值, 返回成功
 *
 */
int SetCommand::operator()(Client *client) const {
    logDebug("execute set command");
    client->initReplyHead(1);
    client->appendReplyBody(string("set no complete yet"));
    return CABINET_OK;
}
#endif
