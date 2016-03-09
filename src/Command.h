#ifndef COMMAND_H
#define COMMAND_H

#ifdef CABINET
    #include "Client.h"
    class Client;
#endif

#include "Const.h"

class Command
{
public:
    Command(): calledTimes(0) {}
#ifdef CABINET
    int operator>>(Client *client) {return CABINET_OK;}
    int operator[](Client *client) {return CABINET_OK;}
    int operator()(Client *client) {return CABINET_OK;}
#endif
    virtual bool isCommandValid() const = 0;
    virtual int commandArgc() const = 0;
    virtual const char commandType() const = 0;
    virtual bool needPF() const = 0;
    int getCalledTimes() const {return calledTimes;}
    virtual ~Command() {}
private:
    int calledTimes;
};

#endif
