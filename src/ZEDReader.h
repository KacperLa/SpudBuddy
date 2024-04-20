#ifndef ZEDREADER_H
#define ZEDREADER_H

#include <chrono>
#include <string>
#include <mutex>
#include <iostream>
#include <atomic>
#include <thread>
#include <cuda_runtime.h>

#include <fcntl.h>
#include <unistd.h>
#include <cmath>

#include<ronThread.h>
#include <shared_structs.h>

#include <libraries/sdata/include/sdata.hpp>

using namespace sdata;

#include <sl/Camera.hpp>

using namespace sl;
class ZEDReader : public ronThread
{
public:
  ZEDReader(const std::string& areaFile, const std::string name, Log* logger);
  virtual ~ZEDReader();
  
  void getIMUData(imuData_t *data);
  void saveMesh();

private:
  virtual void loop() override;

protected:
    void stop();
    bool open();

    std::string device;

    Camera zed;
    std::string m_areaFile{""};

    std::uint64_t  loop_time = (1000000000 / 15); // 15 Hz

    SData<positionSystem_t> shared_tracking_state;
    SData<camera_feed_t>   shared_camera_feed;
    // SData<pointCloud_t>   shared_point_cloud;

    imuData_t imu_state;
};

#endif
