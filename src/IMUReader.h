#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <string>
#include "iostream"
#include <mutex>

struct IMUState {
    std::chrono::high_resolution_clock::time_point timestamp;
    int roll {0};
    int pitch {0};
    int yaw {0};
    bool quality {false};
};

class IMUReader {
public:
    IMUReader(const std::string &device);

    virtual ~IMUReader();

    virtual bool open();

    virtual bool readEvent(js_event &event);

    void getState(IMUState & data);
    void updateState(IMUState data);

    void ReadIMULoop();

    virtual void close();

private:
    std::mutex imu_state_lock;

    std::string device;

    imuState imu_state;
};

#endif