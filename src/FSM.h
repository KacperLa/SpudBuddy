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

struct imu_data : tinyfsm::Event {
    IMUState state;
};

struct cmd_data : tinyfsm::Event {
    double v;
    double o;
};


class Robot 
: public tinyfsm::Fsm<Robot>
{
public:

    void react(tinyfsm::Event const &) { };

    void set_vbusVoltage(float* newVoltage);
    
    void get_vbusVoltage(float& voltage);

    static int get_state() { return current_state; }
    static void set_logger(Log* new_logger) { logger = new_logger;};
    //void process_odrive_heartbeat(uint32_t id, HeartbeatMsg_t heartbeat);

    virtual void entry(void);  /* entry actions in some states */
    virtual void exit(void);  /* no exit actions at all */
    virtual void react(Update const &);
    virtual void react(START const &);
    virtual void react(RESET const &);

    void react(FAIL const &);
    void react(SHUTDOWN const &);

    void react(imu_data const &e);
    void react(cmd_data const &e);
    static json RequestAxisData(int id);
    static bool requestDriveSystemStatus();
    static json getControllerCoeffs();
    static void setControllerCoeffs(json & coeff);

    static RobotState desired_state;
    static RobotState actual_state;

    static constexpr std::chrono::duration<double> error_time{1.0 / 100.0}; // 100 Hz


    static int current_state;

    static Controller controller;
    static DriveSystem driveSystem;
    
    static Log* logger;

    static const int leftNode       {0};
    static const int rightNode      {1};


private:
  

  ODriveCAN::AxisState_t requestedWheelState = ODriveCAN::AXIS_STATE_UNDEFINED;
  
  ODriveCAN odriveCAN;
  
  static float adcVoltage;
  static float vbusVoltage;



  static std::chrono::time_point<std::chrono::steady_clock> clock_last_run;

  unsigned long interval    {20};
};



