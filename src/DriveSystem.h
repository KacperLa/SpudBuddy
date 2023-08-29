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
#include "math.h"

#include "drivers/ODriveCAN/ODriveCAN.h"
#include<ronThread.h>


struct DriveState {
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    float velocity {0};
    float position {0};
    int state {0};
    bool error {0};
};

class DriveSystem : public ronThread
{
public:
    DriveSystem(const int id[], const bool dir[], const int size, const std::string name, Log& logger);
    virtual ~DriveSystem();

    virtual bool open();
    virtual bool readEvent(can_frame& event);
    virtual void close();

    virtual void updateState(const DriveState& data, const int axos_id);

    void setTorque(float& t, const int axis_id);
    void getVelocity(float& vel, const int axis_id);
    void getPosition(float& pos, const int axis_id);

    bool getState(DriveState& data, int axid_id);
    bool getStatus();
    void calcDeadRec(float & x, float & y);

    bool enable(); // ODriveCAN::AxisState_t::AXIS_STATE_CLOSED_LOOP_CONTROL
    bool disable(); // ODriveCAN::AxisState_t::AXIS_STATE_IDLE
    bool ESTOP();
    bool reset();

private:
    virtual void loop() override;

    void runState(int axisState);

    int findIndex(const int arr[], int target, int size);
    
    bool isReversed(int axis_id);

protected:

    ODriveCAN::AxisState_t requestedWheelState = ODriveCAN::AXIS_STATE_UNDEFINED;
    ODriveCAN odriveCAN;
    std::thread read_thread;

    int numberOfNodes {2};

    double deadRecPos[3] {0.0f, 0.0f, 0.0f};
    float lastWheelPos[2] {0.0f, 0.0f};

    float vBusVoltage;

    std::string device;
    
    DriveState state[2];
    const int* nodeIDs;
    const bool* nodeReversed;

};

#endif // JOYSTICK_H