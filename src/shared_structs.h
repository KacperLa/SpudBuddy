#include "drivers/bno055/BNO055.h"

typedef BNO055::euler_angles angles_t;
typedef BNO055::gyro rates_t;
typedef BNO055::bno_cal imu_cal_info_t;
typedef BNO055::bno_info imu_sys_info_t;

#ifndef SHARED_STRUCTS_H
#define SHARED_STRUCTS_H

struct IMUState {
  angles_t angles {0.0, 0.0, 0.0};
  rates_t rates   {0.0, 0.0, 0.0};
  std::int64_t timestamp;
  bool quality {false};
  bool error {false};
};

struct position_t {
        float x {0.0f};
        float y {0.0f};
        float z {0.0f};
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

struct JoystickState {
    float x {0.0f};
    float y {0.0f};
    int time;
};

struct DriveState {
    float velocity {0};
    float position {0};
    float vBusVoltage {0};
    int state {0};
    bool error {0};
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
    int state {0};
    JoystickState joystick;
    position_t position;
};

#endif // SHARED_STRUCTS_H



