#ifndef ZEDREADER_H
#define ZEDREADER_H

#include <chrono>
#include <string>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>

#include <fcntl.h>
#include <unistd.h>
#include <cmath>

#include<ronThread.h>
#include <shared_structs.h>

#include <sl/Camera.hpp>

struct slamState_t {
  float x{0.0f};
  float y{0.0f};
  float z{0.0f};
  bool tracking_state{false};
};

using namespace sl;
class ZEDReader : public ronThread
{
public:
  ZEDReader(const std::string& areaFile, const std::string name, Log* logger);
  virtual ~ZEDReader();

  bool getState(IMUState& data);
  bool getState(slamState_t& data);

private:
  virtual void loop() override;

protected:
    void stop();
    void updateState(IMUState data);
    void updateState(slamState_t data);

    bool open();

    std::string device;

    Camera zed;
    std::string m_areaFile{""};

    std::int64_t  loop_time = (1000000.0 / 400.0); // 400 Hz

    slamState_t slam_state;
    IMUState imu_state;
};

#endif
