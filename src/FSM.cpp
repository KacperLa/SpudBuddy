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
  desired_state.velocity  = e.v;
  desired_state.rates.gyro_yaw = e.w;
}

void Robot::updateIMU(IMUState & new_imu_state)
{
  if (std::chrono::high_resolution_clock::now() - new_imu_state.timestamp > error_time){
    // logger->pushEvent("[FSM] IMU is too old.");
  }
  actual_state.angles = new_imu_state.angles;
  actual_state.rates = new_imu_state.rates;
  imu_timestamp = new_imu_state.timestamp;
}

bool Robot::requestDriveSystemStatus(){ 
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
    double timeInSeconds = 0; //a_s.timestamp.time_since_epoch().count() * std::chrono::high_resolution_clock::period::num / static_cast<double>(std::chrono::high_resolution_clock::period::den);
    json j = {
                  {"axis", id}, 
                  {"state", 
                    {
                      {"position", a_s.position},
                      {"velocity", a_s.velocity},
                      {"error", a_s.error},
                      {"state", a_s.state},
                      {"timestamp", timeInSeconds}
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

robot_state_t Robot::desired_state = {{0.0f, 0.0f},{0.0f, 0.0f},{0.0f, 0.0f}, 0,{0.0, 7.0, 90.0}, {0.0, 0.0, 0.0}, 0.0, 0.0, 0.0};
robot_state_t Robot::actual_state;
Controller Robot::controller;

DriveSystem* Robot::drive_system = {nullptr};
Log* Robot::logger = {nullptr};
std::chrono::high_resolution_clock::time_point Robot::imu_timestamp;
IMUState Robot::imu_state;

class Running : public Robot
{
  void entry() override {
    actual_state.state = 2;
    logger->pushEvent("[FSM] Entering running state.");
    drive_system->enable();
  };
  void exit() override {
    logger->pushEvent("[FSM] Exiting running state.");
    drive_system->disable();
  };
  void react(Update const &) override {
    float leftWheel_cmd;
    float rightWheel_cmd;
    

    drive_system->getVelocity(actual_state.leftVelocity, leftNode);
    drive_system->getVelocity(actual_state.rightVelocity, rightNode);
    actual_state.velocity = (actual_state.leftVelocity+actual_state.rightVelocity) / 2.0f;


    if (fabs(actual_state.angles.pitch - desired_state.angles.pitch) > 50){
      logger->pushEvent("[FSM] Robot fell, going into error state." + std::to_string(actual_state.angles.pitch));
      transit<Error>();
    } else {
      if (controller.calculateOutput(actual_state, desired_state, leftWheel_cmd, rightWheel_cmd)){
        drive_system->setTorque(leftWheel_cmd, leftNode);
        drive_system->setTorque(rightWheel_cmd, rightNode);
      } else {
        logger->pushEvent("[FSM] Robot controller failure\n");
        transit<Error>();
      }
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
    actual_state.state = 3;
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
    actual_state.state = 1;
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