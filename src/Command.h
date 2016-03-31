#ifndef COMMAND_H
#define COMMAND_H

#ifdef CABINET
    #include "Client.h"
    class Client;
#endif

#include "Const.h"
#include "Log.h"

class Command
{
public:
    Command(): calledTimes(0) {}
#ifdef CABINET
    virtual int operator>>(Client *client) const {logFatal("should not use default command >>"); return CABINET_OK;}
    virtual int operator[](Client *client) const {logFatal("should not use default command []"); return CABINET_OK;}
    virtual int operator()(Client *client) const {logFatal("should not use default command ()"); return CABINET_OK;}
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
