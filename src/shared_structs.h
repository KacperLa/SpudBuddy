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
    float pitch_p;
    float pitch_i;
    float pitch_d;
    float velocity_p;
    float velocity_i;
    float velocity_d;
    float yaw_rate_p;
    float yaw_rate_i;
    float yaw_rate_d;
    float pitch_zero;
    float yaw_p;
    float yaw_i;
    float yaw_d;
    float positon_p;
    float positon_i;
    float positon_d;
    float dead_zone;

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

struct sytemState_t {
    robot_state_t actual;
    driveSystemState driveSystem;
    controllerSettings_t controller_settings;
};

struct systemDesired_t {
    int state;
    JoystickState joystick;
    position_t position;
    
    // fill the struct with zeros
    systemDesired_t() noexcept { std::memset(this, 0, sizeof(systemDesired_t)); }
};

#endif // SHARED_STRUCTS_H



