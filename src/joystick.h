#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <chrono>
#include <string>
#include "iostream"
#include <mutex>

struct JoystickState {
    std::chrono::high_resolution_clock::time_point timestamp;
    int x {0};
    int y {0};
};

class Joystick {
public:
    Joystick(const std::string &device);

    virtual ~Joystick();

    virtual bool open();

    virtual bool readEvent(js_event &event);

    void getState(JoystickState & data);
    void updateState(JoystickState data);

    void ReadJoystickLoop();

    virtual void close();

private:
    std::mutex joystick_state_lock;

    std::string device;
    int joystick;

    JoystickState js_state;
};

#endif