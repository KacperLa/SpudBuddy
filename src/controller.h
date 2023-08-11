#include <libraries/MiniPID/MiniPID.h>
#include <cmath>
#include <mutex>
#include <thread>

#include "drivers/bno055/BNO055.h"

typedef BNO055::euler_angles angles_t;
typedef BNO055::gyro rates_t;


struct RobotState
{
    angles_t angles {0.0, 0.0, 0.0};
    rates_t rates {0.0, 0.0, 0.0};
    float velocity {0.0};
    float leftVelocity {0.0};
    float rightVelocity {0.0};
    int state {0};
};

class Controller {
public:
    Controller();
    ~Controller();

    bool calculateOutput(RobotState actual_state, RobotState desired_state, float& outputLeft, float& outputRight);

    void get_pitch_coeffs(double & P, double & I, double & D);
    void get_yaw_rate_coeffs(double & P, double & I, double & D);
    void get_velocity_coeffs(double & P, double & I, double & D);

    void set_pitch_coeffs(const double * P, const double * I, const double * D);
    void set_yaw_rate_coeffs(const double * P, const double * I, const double * D);
    void set_velocity_coeffs(const double * P, const double * I, const double * D);

private:
    std::mutex mtx;

    MiniPID pitch_pid = MiniPID(0.08, 0.0, 0.015);
    MiniPID yaw_rate_pid = MiniPID(0.05, 0, 0.02);
    MiniPID velocity_pid = MiniPID(15, 0.2, 0);


};