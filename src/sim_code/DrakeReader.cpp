#include "DrakeReader.h"
#include <sys/resource.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include "common.h"

StateReader::StateReader(const std::string& areaFile, const std::string name, Log& logger) :
  ronThread(name, logger, false),
  shared_state_map("shared_sim_state_map", logger, shared_sim_state_file, semaphore_sim_state_file, false)
{

}

StateReader::~StateReader()
{
}

bool StateReader::open()
{
    
    return true;
}

bool StateReader::getState(imuData_t& data) {
  std::lock_guard<std::mutex> lock(thread_lock);
  data = imu_state;
  return data.error;
}

bool StateReader::getState(slamState_t& data) {
  std::lock_guard<std::mutex> lock(thread_lock);
  data = slam_state;
  return data.tracking_state;
}

void StateReader::updateState(imuData_t data) {
  std::lock_guard<std::mutex> lock(thread_lock);
  imu_state = data;
}

void StateReader::updateState(slamState_t data) {
  std::lock_guard<std::mutex> lock(thread_lock);
  slam_state = data;
}

void StateReader::stop() {
  running = false;
}

void StateReader::loop() {
    setpriority(PRIO_PROCESS, getpid(), 1);

    imuData_t data;
    slamState_t slam_data;

    // set error to true untill camera is open
    data.error = true;
    updateState(data);

    shared_state_map.startThread();

    while (running) {
      shared_state_map.getData(shared_state);

      angles_t angles = shared_state.imu_state.angles;
      rates_t rates = shared_state.imu_state.rates;
      data = {angles,
              rates,
              get_time_nano(),
              1, 0};
      updateState(data);

      
      slam_data = shared_state.slam_state;
           
      updateState(slam_data);
      
      // log("roll: " + std::to_string(data.angles.roll) + ", pitch: " + std::to_string(data.angles.pitch) + ", yaw: " + std::to_string(data.angles.yaw) + ", x: " + std::to_string(slam_data.x) + ", y: " + std::to_string(slam_data.y) + ", z: " + std::to_string(slam_data.z) + ", tracking: " + std::to_string(slam_data.tracking_state));
      // log("x: " + std::to_string(slam_data.x) + ", y: " + std::to_string(slam_data.y) + ", z: " + std::to_string(slam_data.z) + ", tracking: " + std::to_string(slam_data.tracking_state));
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

