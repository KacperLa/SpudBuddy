#include <libraries/MiniPID/MiniPID.h>
#include <cmath>
#include <mutex>
#include <thread>

#include "drivers/bno055/BNO055.h"
#include <shared_structs.h>


class Controller {
public:
    Controller();
    ~Controller();

    bool calculateOutput(robot_state_t actual_state, robot_state_t desired_state, float& outputLeft, float& outputRight);

    void get_settings(controllerSettings_t & settings);
    void set_settings(const controllerSettings_t & settings);

private:
    std::mutex mtx;
    controllerSettings_t m_settings;

    float max_distance = 0.5f;

    MiniPID pitch_pid    = MiniPID(0.08, 0.0, 0.015);
    MiniPID velocity_pid = MiniPID(13, 0.0, 0.8);
    MiniPID yaw_rate_pid = MiniPID(1.0, 0, 0.0175);
    MiniPID yaw_pid      = MiniPID(0.0, 0.0, 0.0);
    MiniPID position_pid = MiniPID(0.0, 0.0, 0.0);
};