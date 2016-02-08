#include "GetCommand.h"

#ifdef CABINET_SERVER
int GetCommand::operator()(Client &client) const {

    return 0;
}
#endif
