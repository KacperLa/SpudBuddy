#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <string>
#include <libraries/tinyfsm/include/tinyfsm.hpp>
#include "controller.h"
#include "driveSystem.h"
#include <shared_structs.h>
#include <sdata.h>

#include<logger.h>

#include <iostream>
#include <cmath>

#include <libraries/json/json.hpp>
using json = nlohmann::json;

struct Update    : tinyfsm::Event { };
struct START     : tinyfsm::Event { };
struct FAIL      : tinyfsm::Event { };
struct RESET     : tinyfsm::Event { };
struct SHUTDOWN  : tinyfsm::Event { };

typedef enum {
    IDLE = 1,
    RUNNING = 2,
    ERROR = 3
} RobotStates;

struct cmd_data : tinyfsm::Event {
    double v {0.0};
    double w {0.0};
};

class Robot : public tinyfsm::Fsm<Robot>
{
public:

    void react(tinyfsm::Event const &) { };

    void set_vbusVoltage(float* newVoltage);
    
    void get_vbusVoltage(float& voltage);

    static void set_logger(Log* new_logger) { logger = new_logger;};
    static void setDriveSystem(driveSystem* new_drive_system) { drive_system = new_drive_system;};

    virtual void entry(void);  /* entry actions in some states */
    virtual void exit(void);  /* no exit actions at all */
    virtual void react(Update const &);
    virtual void react(START const &);
    virtual void react(RESET const &);

    void react(FAIL const &);
    void react(SHUTDOWN const &);

    void react(cmd_data const &e);
    static json RequestAxisData(int id);
    static bool requestDriveSystemStatus();
    static float getVelocityOfAxis(int axis);
    static bool updateSubsystemStatus();

    static void getControllerSettings(controllerSettings_t & settings);
    static void setControllerSettings(controllerSettings_t & settings);
    static void updateIMU(imuData_t & new_imu_state);

    // static systemState_t desired_state;
    // static systemState_t actual_state;

    static std::uint64_t error_time; // 1000 Hz

    static imuData_t imu_state;

    static Controller controller;
    
    static Log* logger;
    static driveSystem* drive_system;

    static SData<imuData_t>* shared_imu_state;
    static SData<trackingState_t>* shared_tracking_state;
    static SData<driveSystemState_t>* shared_drive_system_state;
    static SData<systemDesired_t>* shared_command_state;    
    static SData<controllerSettings_t>* shared_settings;

    static int state;

private:
  
  ODriveCAN::AxisState_t requestedWheelState = ODriveCAN::AXIS_STATE_UNDEFINED;
  
  ODriveCAN odriveCAN;
  
  static float adcVoltage;
  static float vbusVoltage;


  static std::uint64_t clock_last_run;

  unsigned long interval    {20};
};



