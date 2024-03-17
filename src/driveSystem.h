#ifndef dRIVESYSTEM_H
#define dRIVESYSTEM_H

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
#include "common.h"
#include "shared_structs.h"

#include "drivers/ODriveCAN/ODriveCAN.h"
#include<ronThread.h>
#include <sdata.h>

class driveSystem : public ronThread
{
public:
    driveSystem(const int id[], const bool dir[], const int size, const std::string name, Log* logger);
    virtual ~driveSystem();

    virtual bool open();
    virtual bool readEvent(can_frame& event);
    virtual void close();

    virtual void updateState(const DriveState& data, const int axos_id);

    void setTorque(float& t, const int axis_id);
    void setPosition(float& pos, const int axis_id);
    void getVelocity(float& vel, const int axis_id);
    void getPosition(float& pos, const int axis_id);

    void getDeadReckoningPosition(position_t& pos);

    void requestVbusVoltage();
    void requestDRReset();

    bool getState(DriveState& data, int axid_id);
    bool getStatus();
    void calculateDeadReckoning();

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

    imuData_t imu_state;
    SData<imuData_t> shared_imu_state;

    ODriveCAN::AxisState_t requestedWheelState = ODriveCAN::AXIS_STATE_UNDEFINED;
    ODriveCAN odriveCAN;
    std::thread read_thread;

    int numberOfNodes {2};

    position_t dead_reckoning_position;

    double deadRecAngle {0.0}; 

    float lastWheelPos[2] {0.0f, 0.0f};

    float vBusVoltage;

    std::string device;
    
    SData<driveSystemState_t> shared_state;
    SData<positionSystem_t> dead_reckoning_shared_position;

    DriveState state[4];
    const int* nodeIDs;
    const bool* nodeReversed;

};

#endif // JOYSTICK_H