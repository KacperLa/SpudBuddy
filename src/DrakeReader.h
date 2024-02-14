#ifndef DRAKEREADER_H
#define DRAKEREADER_H

#include <string>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#include<sdata.h>

#include <fcntl.h>
#include <unistd.h>
#include <cmath>

#include<ronThread.h>

struct slamState_t {
  float x{0.0f};
  float y{0.0f};
  float z{0.0f};
  bool tracking_state{false};
};

struct shared_state_t {
  slamState_t slam_state;
  imuData_t imu_state;
};

class StateReader : public ronThread
{
public:
  StateReader(const std::string& areaFile, const std::string name, Log& logger);
  virtual ~StateReader();

  bool getState(imuData_t& data);
  bool getState(slamState_t& data);

private:
  virtual void loop() override;

protected:
    void stop();
    void updateState(imuData_t data);
    void updateState(slamState_t data);

    bool open();

    // location of shared memory
    const char* shared_sim_state_file   = "/tmp/sim_state";

    // location of semaphore
    const char* semaphore_sim_state_file  = "/sim_state_sem";

    SData<shared_state_t> shared_state_map;

    shared_state_t shared_state;

    std::uint64_t time_limit  {1000000000 / 10}; // 20 Hz
    std::uint64_t forget_time {1000000000}; // 1 Hz

    slamState_t slam_state;
    imuData_t imu_state;
};

#endif
