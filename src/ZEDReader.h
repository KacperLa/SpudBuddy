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
#include<IMUReader.h>

#include <sl/Camera.hpp>


using namespace sl;
class ZEDReader : public ronThread
{
public:
  ZEDReader(const std::string& areaFile, const std::string name, Log& logger);
  virtual ~ZEDReader();

  bool getState(IMUState& data);

private:
  virtual void loop() override;

protected:
    void stop();
    void updateState(IMUState data);

    bool open();

    std::string device;

    Camera zed;
    std::string m_areaFile{""};

    std::chrono::duration<double> time_limit{1.0 / 10.0}; // 20 Hz
    std::chrono::duration<double> forget_time{1.0 / 1.0}; // 1 Hz

    IMUState imu_state;
};

#endif
