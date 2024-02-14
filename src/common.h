#ifndef COMMON_H
#define COMMON_H

#include <chrono>
#include <cstdint>

using namespace std::chrono;

inline std::uint64_t get_time_nano()
{
    return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
}

const int leftNode       {0};
const int rightNode      {1};

// location of shared memory
const std::string shared_drive_system_file {"robot_drive_system"};
const std::string shared_desired_file {"robot_desired"};
const std::string shared_settings_file {"robot_settings"};

const std::string shared_imu_file      {"robot_imu"};
const std::string shared_tracking_state_file {"robot_tracking"};
const std::string shared_command_file  {"robot_command"};
const std::string shared_actual_state_file  {"robot_actual"};

#endif // COMMON_H
