#ifndef COMMAND_H
#define COMMAND_H

class Command
{
public:
    Command(): calledTimes(0) {}
    virtual int operator()(/*Client *client*/) const = 0;
    int getCalledTimes() const {return calledTimes;}
    virtual ~Command() {}
private:
    int calledTimes = 0;
};

#endif
