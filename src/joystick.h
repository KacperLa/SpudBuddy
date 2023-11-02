#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <linux/joystick.h>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include "fcntl.h"
#include "unistd.h"
#include <poll.h>

#include<ronThread.h>

struct JoystickState {
    float x {0.0f};
    float y {0.0f};
    int time;
};

class Joystick : public ronThread
{
public:
    Joystick(const std::string name, Log& logger);

    void addDevice(const std::string new_device);
    void getState(JoystickState& data);

protected:
    virtual void loop() override;
    bool open();
    void close();

    bool readEvent(js_event& event);

    void updateState(const JoystickState& data);

    std::string device;
    int joystick_;
    struct pollfd pfd;


    JoystickState js_state;
};

#endif // JOYSTICK_H