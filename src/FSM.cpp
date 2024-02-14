#include "FSM.h"

ODriveCAN odriveCAN;

void Robot::entry() {
   logger->pushEvent("[FSM] undefined_state");
}

void Robot::exit() {
   logger->pushEvent("[FSM] undefined_state");
}

void Robot::react(Update const &) {
   logger->pushEvent("[FSM] undefined_state");
}

void Robot::react(START const &) {
   logger->pushEvent("[FSM] undefined_state");
}

void Robot::react(RESET const &) {
   logger->pushEvent("[FSM] undefined_state");
}

bool Robot::requestDriveSystemStatus()
{

  return drive_system->getStatus();
}

float Robot::getVelocityOfAxis(int axis){
  float vel;
  drive_system->getVelocity(vel, axis);
  return vel;
}

json Robot::RequestAxisData(int id)
{
    DriveState a_s;
    drive_system->getState(a_s, id);
    json j = {
                  {"axis", id}, 
                  {"state", 
                    {
                      {"position", a_s.position},
                      {"velocity", a_s.velocity},
                      {"error", a_s.error},
                      {"state", a_s.state},
                      {"timestamp", get_time_nano()}
                    }
                  }
               };
  
  return j;
}

void Robot::getControllerSettings(controllerSettings_t & settings){
  controller.get_settings(settings);
}

void Robot::setControllerSettings(controllerSettings_t & settings){
  controller.set_settings(settings);
}

class Idle;
class Error;
class Running;

Controller Robot::controller;

driveSystem* Robot::drive_system = {nullptr};
Log* Robot::logger = {nullptr};
std::uint64_t  Robot::error_time = (1000000000 / 390); // 400 Hz

imuData_t Robot::imu_state;

int Robot::state = 0;

SData<imuData_t>*  Robot::shared_imu_state = {nullptr};
SData<trackingState_t>* Robot::shared_tracking_state = {nullptr};
SData<driveSystemState_t>* Robot::shared_drive_system_state = {nullptr};
SData<systemDesired_t>* Robot::shared_command_state = {nullptr};
SData<controllerSettings_t>* Robot::shared_settings = {nullptr};

class Running : public Robot
{
  void entry() override {
    state = 2;
    drive_system->enable();
  };
  void exit() override {
    logger->pushEvent("[FSM] Exiting running state.");
    drive_system->disable();
  };
  void react(Update const &) override {
    static float leftWheel_cmd;
    static float rightWheel_cmd;

    static trackingState_t tracking_state;
    static systemDesired_t command;
    static imuData_t imu_state;
    static driveSystemState_t drive_system_state;
    static controllerSettings_t settings;

    // Block until the IMU data is updated
    if (!shared_imu_state->waitOnStateChange(imu_state))
    {
      logger->pushEvent("[FSM] IMU is in error. Entering error state.");
      transit<Error>();
    }
    // check is imu data is stale
    if ((get_time_nano() - imu_state.timestamp) > (1000000000/100))
    {
      std::uint64_t time_diff = get_time_nano() - imu_state.timestamp;
      logger->pushEvent("[FSM] IMU sample stale sample is : " + std::to_string(time_diff) + " ns old");
      transit<Error>();
    } 

    // get drive system state
    shared_drive_system_state->getData(drive_system_state);
    shared_tracking_state->getData(tracking_state);
    shared_command_state->getData(command);
    shared_settings->getData(settings);

    if (controller.calculateOutput(drive_system_state, imu_state, tracking_state, command, settings, leftWheel_cmd, rightWheel_cmd)){
      drive_system->setTorque(leftWheel_cmd, leftNode);
      drive_system->setTorque(rightWheel_cmd, rightNode);
    } else {
      logger->pushEvent("[FSM] Robot controller failure\n");
      transit<Error>();
    }
   
  };
  void react(START const &) override {
    logger->pushEvent("[FSM] ALREADY IN RUNNING STATE.");
  };
  void react(RESET const &) override  {
    transit<Idle>();
  }
};

class Error : public Robot
{
  void entry() override {
    state = 3;
    logger->pushEvent("[FSM] Entering error state.");
    drive_system->ESTOP();
  };  
  void exit () override {
    logger->pushEvent("[FSM] Exiting error state.");
    drive_system->reset();
  };
  void react(Update const &) override {
    // logger->pushEvent("[FSM] error_state");
    // a cool idea would be too beep at a fixed rate here
  };
  void react(START const &) override  {
    logger->pushEvent("[FSM] UNABLE TO TRANSITION TO RUNNING FROM ERROR.");
  };
  void react(RESET const &) override {
    transit<Idle>();
  }
};

class Idle : public Robot
{
  void entry() override {
    state = 1;
    logger->pushEvent("[FSM] Entering idle state.");
    //play_melody(idle_enter_melody, idle_enter_noteDurations);
    drive_system->disable();
  };
  void exit() override {
    logger->pushEvent("[FSM] Exitting idle.");
  };
  void react(Update const &) override {
    // logger->pushEvent("[FSM] idle_state\n");
  };
  void react(START const &) override  {
    transit<Running>();
  }
  void react(RESET const &) override {
    transit<Idle>();
  }
};


void Robot::react(FAIL const &)
{
  logger->pushEvent("[FSM] Fail state triggered.");
  transit<Error>();
}

void Robot::react(SHUTDOWN const &)
{
  transit<Idle>();
}

FSM_INITIAL_STATE(Robot, Idle)