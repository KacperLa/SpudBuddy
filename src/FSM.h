#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <string>
#include <libraries/tinyfsm/include/tinyfsm.hpp>
#include "IMUReader.h"
#include "controller.h"
#include "DriveSystem.h"

#include<Logging.h>

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


class Robot 
: public tinyfsm::Fsm<Robot>
{
public:

    void react(tinyfsm::Event const &) { };

    void set_vbusVoltage(float* newVoltage);
    
    void get_vbusVoltage(float& voltage);

    static robot_state_t get_state() { return actual_state; }
    static robot_state_t get_desired_state() { return desired_state; }
    static void set_logger(Log* new_logger) { logger = new_logger;};
    static void setDriveSystem(DriveSystem* new_drive_system) { drive_system = new_drive_system;};

    //void process_odrive_heartbeat(uint32_t id, HeartbeatMsg_t heartbeat);

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

    static json getControllerCoeffs();
    static void setControllerCoeffs(json & coeff);
    static void updateIMU(IMUState & new_imu_state);

    static robot_state_t desired_state;
    static robot_state_t actual_state;

    static constexpr std::chrono::duration<double> error_time{1.0 / 50.0}; // 50 Hz


    static IMUState imu_state;

    static Controller controller;
    
    static Log* logger;
    static DriveSystem* drive_system;

    static const int leftNode       {0};
    static const int rightNode      {1};

    static std::chrono::high_resolution_clock::time_point imu_timestamp;


private:
  
  ODriveCAN::AxisState_t requestedWheelState = ODriveCAN::AXIS_STATE_UNDEFINED;
  
  ODriveCAN odriveCAN;
  
  static float adcVoltage;
  static float vbusVoltage;


  static std::chrono::time_point<std::chrono::steady_clock> clock_last_run;

  unsigned long interval    {20};
};



