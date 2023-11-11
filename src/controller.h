#include <libraries/MiniPID/MiniPID.h>
#include <cmath>
#include <mutex>
#include <thread>

#include "drivers/bno055/BNO055.h"

typedef BNO055::euler_angles angles_t;
typedef BNO055::gyro rates_t;

struct position_t {
        float x {0.0f};
        float y {0.0f};
    };

struct robot_state_t
{
    position_t position;
    position_t positionDeadReckoning;
    position_t positionSlam;
    int positionStatus {0};
    angles_t angles {0.0, 0.0, 0.0};
    rates_t rates {0.0, 0.0, 0.0};
    float velocity {0.0f};
    float leftVelocity {0.0f};
    float rightVelocity {0.0f};
    int state {0};
};

struct controllerSettings_t
{
    float pitch_p {0.0f};
    float pitch_i {0.0f};
    float pitch_d {0.0f};
    float velocity_p {0.0f};
    float velocity_i {0.0f};
    float velocity_d {0.0f};
    float yaw_rate_p {0.0f};
    float yaw_rate_i {0.0f};
    float yaw_rate_d {0.0f};
    float pitch_zero {0.0f};
    float yaw_p {0.0f};
    float yaw_i {0.0f};
    float yaw_d {0.0f};
    float positon_p {0.0f};
    float positon_i {0.0f};
    float positon_d {0.0f};
    float dead_zone {0.0f};
};


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

    MiniPID pitch_pid    = MiniPID(0.08, 0.0, 0.015);
    MiniPID velocity_pid = MiniPID(13, 0.3, 0.6);
    MiniPID yaw_rate_pid = MiniPID(0.065, 0, 0.0175);
    MiniPID yaw_pid      = MiniPID(0.0, 0.0, 0.0);
    MiniPID position_pid = MiniPID(0.0, 0.0, 0.0);
};