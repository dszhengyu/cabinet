#ifndef REPLYREQUESTVOTECOMMAND_H
#define REPLYREQUESTVOTECOMMAND_H

#ifdef CABINET
    #include "Client.h"
    class Client;
#endif

#include "Command.h"
class Command;

class ReplyRequestVoteCommand: public Command
{
public:
#ifdef CABINET
    int operator[](Client *client) const;
#endif
    bool isCommandValid() const {return true;}
    int commandArgc() const {return 2;}
    const char commandType() const {return 'r';}
    bool needPF() const {return false;}
};

#endif

