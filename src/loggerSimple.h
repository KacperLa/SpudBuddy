#ifndef SIMPLE_LOGGER_H
#define SIMPLE_LOGGER

#include <logger.h>

class LogSimple : public Log
{
public:
    LogSimple() ;
    virtual ~LogSimple();

    virtual void pushEvent(std::string event)
    {
        std::cout << event << std::endl;
    }
};


#endif // SIMPLE_LOGGER_H