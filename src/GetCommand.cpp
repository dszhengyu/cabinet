#include "GetCommand.h"
#include "Log.h"

#ifdef CABINET_SERVER
int GetCommand::operator()(Client *client) const {
    logDebug("execute get command");
    client->initReplyHead(1);
    client->appendReplyBody(string("get no complete yet"));
    return CABINET_OK;
}
#endif
