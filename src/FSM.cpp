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

void Robot::react(cmd_data const &e)
{
  desired_state.robot.velocity  = e.v;
  desired_state.robot.rates.gyro_yaw = e.w;
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
                      {"timestamp", get_time_micro()}
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

systemState_t Robot::desired_state;
systemState_t Robot::actual_state;
Controller Robot::controller;

DriveSystem* Robot::drive_system = {nullptr};
Log* Robot::logger = {nullptr};
std::int64_t  Robot::error_time = (1000000.0 / 390.0); // 400 Hz

imuData_t Robot::imu_state;

class Running : public Robot
{
  void entry() override {
    actual_state.robot.state = 2;
    if (updateSubsystemStatus())
    {
      logger->pushEvent("[FSM] Entering running state.");
      drive_system->enable();
    }
  };
  void exit() override {
    logger->pushEvent("[FSM] Exiting running state.");
    drive_system->disable();
  };
  void react(Update const &) override {
    float leftWheel_cmd;
    float rightWheel_cmd;

    if (controller.calculateOutput(actual_state, desired_state, leftWheel_cmd, rightWheel_cmd)){
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
    actual_state.robot.state = 3;
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
    actual_state.robot.state = 1;
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

bool Robot::updateSubsystemStatus()
{
  // get drive system status
  drive_system->getState(actual_state.drive_system.axis_0, 0);
  drive_system->getState(actual_state.drive_system.axis_1, 1);

  if (drive_system->getStatus() == false)
  {
    logger->pushEvent("[FSM] Drive system is in error. Entering error state.");
    return false;
  }
  return true;
}

void Robot::updateIMU(imuData_t & new_imu_state)
{
  // if ((get_time_micro() - new_imu_state.timestamp) > error_time)
  // {
  //   logger->pushEvent("[FSM] IMU is too old.");
  // }

  // Check if the IMU is in error
  // if (new_imu_state.error &&
  //     (actual_state.robot.state != RobotStates::ERROR || 
  //     actual_state.robot.state != RobotStates::IDLE))
  // {
  //   logger->pushEvent("[FSM] IMU is in error. Entering error state.");
  //   dispatch(FAIL());
  // }
  


  actual_state.robot.angles = new_imu_state.angles;
  actual_state.robot.rates = new_imu_state.rates;
  // logger->pushEvent("roll: " + std::to_string(actual_state.robot.angles.roll) + ", pitch: " + std::to_string(actual_state.robot.angles.pitch) + ", yaw: " + std::to_string(actual_state.robot.angles.yaw));// + ", x: " + std::to_string(actual_state.position.x) + ", y: " + std::to_string(actual_state.position.y) + ", z: " + std::to_string(actual_state.position.z) + ", tracking: " + std::to_string(actual_state.positionStatus));    
}

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