#include "GetCommand.h"

#ifdef CABINET_SERVER
int GetCommand::operator()(Client &client) const {
    client.initReplyHead(1);
    client.appendReplyBody(string("get no complete yet"));
    return CABINET_OK;
}
#endif
