#ifndef REPLYAPPENDENTRYCOMMAND_H
#define REPLYAPPENDENTRYCOMMAND_H

#ifdef CABINET
    #include "Client.h"
    class Client;
#endif

#include "Command.h"
class Command;

class ReplyAppendEntryCommand: public Command
{
public:
#ifdef CABINET
    int operator[](Client *client) const;
#endif
    bool isCommandValid() const {return true;}
    int commandArgc() const {return 4;}
    const char commandType() const {return 'r';}
    bool needPF() const {return false;}
};

#endif

