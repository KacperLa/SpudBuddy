#ifndef LOGGER_H
#define LOGGER_H

class Log {
public:
    virtual ~Log() = default;

    virtual void pushEvent(std::string event) = 0;
    
};


#endif // SIMPLE_LOGGER_H