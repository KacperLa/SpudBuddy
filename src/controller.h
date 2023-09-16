#include <libraries/MiniPID/MiniPID.h>
#include <cmath>
#include <mutex>
#include <thread>

#include "drivers/bno055/BNO055.h"

typedef BNO055::euler_angles angles_t;
typedef BNO055::gyro rates_t;


struct robot_state_t
{
    struct position {
        float x {0.0f};
        float y {0.0f};
    } position;
    angles_t angles {0.0, 0.0, 0.0};
    rates_t rates {0.0, 0.0, 0.0};
    float velocity {0.0f};
    float leftVelocity {0.0f};
    float rightVelocity {0.0f};
    int state {0};
};

class Controller {
public:
    Controller();
    ~Controller();

    bool calculateOutput(robot_state_t actual_state, robot_state_t desired_state, float& outputLeft, float& outputRight);

    void get_pitch_coeffs(double & P, double & I, double & D);
    void get_yaw_rate_coeffs(double & P, double & I, double & D);
    void get_velocity_coeffs(double & P, double & I, double & D);
    void get_sync_coeffs(double & P, double & I, double & D);

    void set_pitch_coeffs(const double * P, const double * I, const double * D);
    void set_yaw_rate_coeffs(const double * P, const double * I, const double * D);
    void set_velocity_coeffs(const double * P, const double * I, const double * D);
    void set_sync_coeffs(const double * P, const double * I, const double * D);

private:
    std::mutex mtx;

    MiniPID pitch_pid    = MiniPID(0.08, 0.0, 0.015);
    MiniPID velocity_pid = MiniPID(13, 0.3, 0.6);
    MiniPID sync_pid     = MiniPID(0.0, 0.0, 0.0);
    MiniPID yaw_rate_pid = MiniPID(0.065, 0, 0.0175);
};