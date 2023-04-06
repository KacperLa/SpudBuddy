#ifndef DRIVESYSTEM_H
#define DRIVESYSTEM_H

#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include "fcntl.h"
#include "unistd.h"
#include <iterator>

#include "drivers/ODriveCAN/ODriveCAN.h"


struct DriveState {
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    float velocity {0};
    float position {0};
    int state {0};
    bool error {0};
};

class DriveSystem {
public:
    DriveSystem(const int id[], const int size);
    virtual ~DriveSystem();

    virtual bool open();
    virtual bool readEvent(can_frame& event);
    virtual void close();

    virtual void startReading();
    virtual void stopReading();
    virtual void updateState(const DriveState& data, const int axos_id);

    void setTorque(float& t, const int axis_id);
    void getVelocity(float& vel, const int axis_id);
    bool getState(DriveState& data, int axid_id);
    bool getStatus();

    bool enable(); // ODriveCAN::AxisState_t::AXIS_STATE_CLOSED_LOOP_CONTROL
    bool disable(); // ODriveCAN::AxisState_t::AXIS_STATE_IDLE
    bool ESTOP();
    bool reset();

private:
    void runState(int axisState);

    int findIndex(const int arr[], int size, int target);

protected:

    ODriveCAN::AxisState_t requestedWheelState = ODriveCAN::AXIS_STATE_UNDEFINED;
    ODriveCAN odriveCAN;
    std::atomic<bool> running {false};
    std::thread read_thread;

    int numberOfNodes {2};

    float vBusVoltage;

    virtual void readEventLoop();

    std::string device;
    std::mutex state_lock_;
    
    DriveState state[2];
    const int* nodeIDs;

};

#endif // JOYSTICK_H