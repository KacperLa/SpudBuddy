#include <cstring> 

#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

struct angles_t
{
    float roll;
    float pitch;
    float yaw;

    // fill the struct with zeros
    angles_t() noexcept { std::memset(this, 0, sizeof(angles_t)); }
    angles_t(float roll, float pitch, float yaw) : roll(roll), pitch(pitch), yaw(yaw) {}
};

struct rates_t
{
    float gyro_roll;
    float gyro_pitch;
    float gyro_yaw;

    // fill the struct with zeros
    rates_t() noexcept { std::memset(this, 0, sizeof(rates_t)); }
    rates_t(float gyro_rol, float gyro_pitch, float gyro_yaw) : gyro_roll(gyro_rol), gyro_pitch(gyro_pitch), gyro_yaw(gyro_yaw) {}
};

struct IMUState
{
  angles_t angles ;
  rates_t rates;
  std::int64_t timestamp;
  bool quality;
  bool error;

  // fill the struct with zeros
  IMUState() noexcept { std::memset(this, 0, sizeof(IMUState)); }
  IMUState(angles_t angles, rates_t rates, std::int64_t timestamp, bool quality, bool error) : angles(angles), rates(rates), timestamp(timestamp), quality(quality), error(error) {}
};

struct position_t
{
    float x;
    float y;
    float z;

    // fill the struct with zeros
    position_t() noexcept { std::memset(this, 0, sizeof(position_t)); }
    position_t(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct robot_state_t
{
    position_t position;
    position_t positionDeadReckoning;
    position_t positionSlam;
    int positionStatus;
    angles_t angles;
    rates_t rates;
    float velocity;
    float leftVelocity;
    float rightVelocity;
    int state;

    // fill the struct with zeros
    robot_state_t() noexcept { std::memset(this, 0, sizeof(robot_state_t)); }
    robot_state_t(position_t position, position_t positionDeadReckoning, position_t positionSlam, int positionStatus, angles_t angles, rates_t rates, float velocity, float leftVelocity, float rightVelocity, int state) : position(position), positionDeadReckoning(positionDeadReckoning), positionSlam(positionSlam), positionStatus(positionStatus), angles(angles), rates(rates), velocity(velocity), leftVelocity(leftVelocity), rightVelocity(rightVelocity), state(state) {}
};

struct controllerSettings_t
{
    double pitch_p;
    double pitch_i;
    double pitch_d;
    double velocity_p;
    double velocity_i;
    double velocity_d;
    double yaw_rate_p;
    double yaw_rate_i;
    double yaw_rate_d;
    double pitch_zero;
    double yaw_p;
    double yaw_i;
    double yaw_d;
    double positon_p;
    double positon_i;
    double positon_d;
    double dead_zone_pos;
    double dead_zone_yaw;
    double dead_zone_vel;
    double dead_zone_pitch;
    double dead_zone_yaw_rate;

    // fill the struct with zeros
    controllerSettings_t() noexcept { std::memset(this, 0, sizeof(controllerSettings_t)); }
};

struct JoystickState {
    float x;
    float y;
    int time;

    // fill the struct with zeros
    JoystickState() noexcept { std::memset(this, 0, sizeof(JoystickState)); }
};

struct DriveState {
    float velocity;
    float position;
    float vBusVoltage;
    int state;
    bool error;

    // fill the struct with zeros
    DriveState() noexcept { std::memset(this, 0, sizeof(DriveState)); }
};

// DriveSystem specific
struct driveSystemState {
    DriveState axis_0;
    DriveState axis_1;
};

struct systemState_t {
    robot_state_t        robot;
    driveSystemState     drive_system;
    controllerSettings_t controller;
};

struct systemDesired_t {
    int state;
    JoystickState joystick;
    position_t position;
    
    // fill the struct with zeros
    systemDesired_t() noexcept { std::memset(this, 0, sizeof(systemDesired_t)); }
};

#endif // SHARED_STRUCTS_H



