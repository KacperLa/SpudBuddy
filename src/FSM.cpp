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
  //desired_state.velocity  = e.v;
  desired_state.rates.gyro_x = e.o;
}

void Robot::react(imu_data const &e)
{
  if (std::chrono::high_resolution_clock::now() - e.state.timestamp > error_time){
    logger->pushEvent("[FSM] IMU is too old.");
  }
  actual_state.angles = e.state.angles;
  actual_state.rates = e.state.rates;
}

bool Robot::requestDriveSystemStatus(){ 
  return driveSystem.getStatus();
}

json Robot::RequestAxisData(int id)
{
    DriveState a_s;
    driveSystem.getState(a_s, id);
    double timeInSeconds = a_s.timestamp.time_since_epoch().count() * std::chrono::high_resolution_clock::period::num / static_cast<double>(std::chrono::high_resolution_clock::period::den);
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

json Robot::getControllerCoeffs(){
    double pP, pI, pD, vP, vI, vD, yP, yI, yD;

    controller.get_pitch_coeffs(pP, pI, pD);
    controller.get_yaw_rate_coeffs(yP, yI, yD);
    controller.get_velocity_coeffs(vP, vI, vD);
    json j = {
                  {"pitch", 
                    {
                      {"p", pP},
                      {"i", pI},
                      {"d", pD}
                    }
                  },
                  {"yaw", 
                    {
                      {"p", yP},
                      {"i", yI},
                      {"d", yD}
                    }
                  },
                  {"velocity", 
                    {
                      {"p", vP},
                      {"i", vI},
                      {"d", vD}
                    }
                  },
               };
  
  return j;
}

void Robot::setControllerCoeffs(json & coeffs){
    const double pP = std::stod(static_cast<std::string>(coeffs.at("pP")));
    const double pI = std::stod(static_cast<std::string>(coeffs.at("pI")));
    const double pD = std::stod(static_cast<std::string>(coeffs.at("pD")));
    const double vP = std::stod(static_cast<std::string>(coeffs.at("vP")));
    const double vI = std::stod(static_cast<std::string>(coeffs.at("vI")));
    const double vD = std::stod(static_cast<std::string>(coeffs.at("vD")));
    const double yP = std::stod(static_cast<std::string>(coeffs.at("yP")));
    const double yI = std::stod(static_cast<std::string>(coeffs.at("yI")));
    const double yD = std::stod(static_cast<std::string>(coeffs.at("yD")));

    controller.set_pitch_coeffs(&pP, &pI, &pD);
    controller.set_yaw_rate_coeffs(&yP, &yI, &yD);
    controller.set_velocity_coeffs(&vP, &vI, &vD);
}


class Idle;
class Error;
class Running;

int Robot::current_state = 0;
RobotState Robot::desired_state = {{0.0, 10.0, 0.0}, {0.0, 0.0, 0.0}, {0.0}, {0.0}, {0.0}};
RobotState Robot::actual_state;
Controller Robot::controller;
int nodes[2] = {Robot::leftNode, Robot::rightNode};
DriveSystem Robot::driveSystem(nodes, 2);
Log* Robot::logger;

class Running : public Robot
{
  void entry() override {
    current_state = 2;
    logger->pushEvent("[FSM] Entering running state.");
    driveSystem.enable();
  };
  void exit() override {
    logger->pushEvent("[FSM] Exiting running state.");
    driveSystem.disable();
  };
  void react(Update const &) override {
    float leftWheel_cmd;
    float rightWheel_cmd;
    driveSystem.getVelocity(actual_state.leftVelocity, leftNode);
    driveSystem.getVelocity(actual_state.rightVelocity, rightNode);
   
    if (fabs(actual_state.angles.pitch) > 30){
      logger->pushEvent("[FSM] Robot fell, going into error state.");
      transit<Error>();
    } else {
      if (controller.calculateOutput(actual_state, desired_state, leftWheel_cmd, rightWheel_cmd)){
        driveSystem.setTorque(leftWheel_cmd, leftNode);
        driveSystem.setTorque(rightWheel_cmd, rightNode);
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
    current_state = 3;
    logger->pushEvent("[FSM] Entering error state.");
    driveSystem.ESTOP();
  };  
  void exit () override {
    logger->pushEvent("[FSM] Exiting error state.");
    driveSystem.reset();
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
    current_state = 1;
    logger->pushEvent("[FSM] Entering idle state.");
    //play_melody(idle_enter_melody, idle_enter_noteDurations);
    driveSystem.disable();
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
  driveSystem.stopReading();
}

FSM_INITIAL_STATE(Robot, Idle)