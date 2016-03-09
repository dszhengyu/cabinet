#include "NoMatchCommand.h"

#ifdef CABINET
int NoMatchCommand::operator()(Client *client) const {
    client->initReplyHead(1);
    client->appendReplyBody(string("no match command"));
    return CABINET_OK;
}
#endif
