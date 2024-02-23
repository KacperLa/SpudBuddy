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

struct imuData_t
{
  angles_t angles;
  rates_t rates;
  std::uint64_t timestamp;
  bool quality;
  bool error;

  // fill the struct with zeros
  imuData_t() noexcept { std::memset(this, 0, sizeof(imuData_t)); }
  imuData_t(angles_t angles, rates_t rates, std::uint64_t timestamp, bool quality, bool error) : angles(angles), rates(rates), timestamp(timestamp), quality(quality), error(error) {}
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

struct systemActual_t
{
    int state;

    // fill the struct with zeros
    systemActual_t() noexcept { std::memset(this, 0, sizeof(systemActual_t)); }
    systemActual_t(int state) : state(state) {}
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

// Tracking specific
struct trackingState_t {
    position_t position;
    bool is_tracking;
    time_t timestamp;

    // fill the struct with zeros
    trackingState_t() noexcept { std::memset(this, 0, sizeof(trackingState_t)); }
    trackingState_t(position_t position, bool is_tracking, time_t timestamp) : position(position), is_tracking(is_tracking), timestamp(timestamp) {}
};

// DriveSystem specific
struct driveSystemState_t {
    // array of axis
    position_t position;
    DriveState axis[4];

    // fill the struct with zeros
    driveSystemState_t() noexcept { std::memset(this, 0, sizeof(driveSystemState_t)); }

    // function to accees each axis by index 
    DriveState getAxis(const std::uint8_t index) { return axis[index]; }
};

struct systemDesired_t {
    int state;
    position_t position;
    angles_t angles;
    rates_t rates;
    float leftShoulder;
    float rightShoulder;
    
    // fill the struct with zeros
    systemDesired_t() noexcept { std::memset(this, 0, sizeof(systemDesired_t)); }
};

#endif // SHARED_STRUCTS_H



