#ifndef COMMON_H
#define COMMON_H

#include <chrono>
#include <cstdint>

using namespace std::chrono;

inline std::uint64_t get_time_nano()
{
    return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
}

inline nanoseconds get_time_nano_chrono()
{
    return duration_cast<nanoseconds>(system_clock::now().time_since_epoch());
}

inline std::uint64_t nano_to_uint64(nanoseconds time)
{
    return duration_cast<nanoseconds>(time).count();
}

const float max_position[4] {0.0f, 0.0f,  2.30f,  3.0f}; // max position of each node 0 means no limit
const float min_position[4] {0.0f, 0.0f, -2.30f, -3.0f}; // min position of each node 0 means no limit


const int leftNode       {0};
const int rightNode      {1};
const int leftShoulder   {2};
const int rightShoulder  {3};

// location of shared memory
const std::string shared_drive_system_file {"robot_drive_system"};
const std::string shared_desired_file {"robot_desired"};
const std::string shared_settings_file {"robot_settings"};
const std::string shared_camera_feed_file {"robot_camera"};
const std::string shared_dead_reckoning_file {"robot_dead_reckoning"};

const std::string shared_imu_file      {"robot_imu"};
const std::string shared_tracking_state_file {"robot_tracking"};
const std::string shared_command_file  {"robot_command"};
const std::string shared_actual_state_file  {"robot_actual"};

#endif // COMMON_H
