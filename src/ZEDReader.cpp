#include "ZEDReader.h"
#include <sys/resource.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include "common.h"

ZEDReader::ZEDReader(const std::string& areaFile, const std::string name, Log* logger) :
  ronThread(name, logger)
{
    m_areaFile = areaFile;
}

ZEDReader::~ZEDReader()
{
}

bool ZEDReader::open() {
    // Set configuration parameters for the ZED
    InitParameters init_parameters;
    init_parameters.coordinate_units = UNIT::METER;
    init_parameters.coordinate_system = COORDINATE_SYSTEM::RIGHT_HANDED_Z_UP;
    init_parameters.sdk_verbose = true;
    init_parameters.camera_fps = 15;
    init_parameters.camera_resolution = RESOLUTION::HD720;
    init_parameters.depth_mode = DEPTH_MODE::ULTRA;
    
    PositionalTrackingParameters positional_tracking_param;  
    positional_tracking_param.enable_imu_fusion = true;
    // positional_tracking_param.enable_area_memory = true;
    // Open the camera
    auto returned_state = zed.open(init_parameters);
    if (returned_state != ERROR_CODE::SUCCESS)
    {
        // log("Camera Open " + std::to_string(returned_state) + " exit thread.");
        return false;
    }

    returned_state = zed.enablePositionalTracking(positional_tracking_param);
    if (returned_state != ERROR_CODE::SUCCESS)
    {
        log("Enabling positionnal tracking failed");
        zed.close();
        return false;
    }
    return true;
}

bool ZEDReader::getState(slamState_t& data) {
  std::memcpy(&data, &slam_state[(update_count.load(std::memory_order_acquire) + 1) % 2], sizeof(slamState_t));
  return data.tracking_state; 
}

void ZEDReader::updateState(slamState_t data) {
  std::memcpy(&slam_state[update_count.load(std::memory_order_acquire) % 2], &data, sizeof(slamState_t));
  update_count.fetch_add(1, std::memory_order_release);
}

void ZEDReader::stop() {
  running = false;
}

void ZEDReader::getIMUData(imuData_t& data)
{
    SensorsData sensors_data;
    if (zed.getSensorsData(sensors_data, TIME_REFERENCE::CURRENT ) == sl::ERROR_CODE::SUCCESS)
    {
        auto zedAngles = sensors_data.imu.pose.getEulerAngles(true);
        auto zedRates = sensors_data.imu.angular_velocity;
        data = {{-1.0f*zedAngles[0], -1.0f*zedAngles[1], zedAngles[2]},
                            {-1.0f*zedRates[0],  -1.0f*zedRates[1],  zedRates[2]},
                            get_time_micro(), 1, 0};
        // log("yaw: " + std::to_string(data.angles.yaw) + " pitch: " + std::to_string(data.angles.pitch) + " roll: " + std::to_string(data.angles.roll));
    }
    else
    {
        data.error = true;
    }
}

void ZEDReader::loop() {
    setpriority(PRIO_PROCESS, getpid(), 1);

    slamState_t tracking_data;
    SensorsData sensors_data;
    Pose camera_path;
    POSITIONAL_TRACKING_STATE tracking_state;

    // set error to true untill camera is open
    tracking_data.tracking_state = false;
    tracking_data.imu.error = true;
    updateState(tracking_data);

    // Open the ZED device
    if (open() != 1) {
        std::cerr << "Error opening ZED." << std::endl;
        tracking_data.imu.error = true;
        return;
    }

    while (running) {
        auto last_run = get_time_micro();
        if (zed.grab() == ERROR_CODE::SUCCESS)
        {
            // Get the position of the camera in a fixed reference frame (the World Frame)
            tracking_state = zed.getPosition(camera_path, REFERENCE_FRAME::WORLD);

            if (tracking_state == POSITIONAL_TRACKING_STATE::OK)
            {
                tracking_data.position = {camera_path.getTranslation().tx, 
                                          camera_path.getTranslation().ty, 
                                          camera_path.getTranslation().tz, 
                                        };
                tracking_data.tracking_state = true;
            } else {
                tracking_data.tracking_state = false;
            }
            // log("ZEDReader: " + std::to_string(data.angles.roll) + " " + std::to_string(data.angles.pitch) + " " + std::to_string(data.angles.yaw) + " " + std::to_string(slam_data.tracking_state));
            updateState(tracking_data);
        }  
        else
        {
            // watch dog set error is no new readingin a while
            // std::cerr << "Error getting ZED data." << std::endl;
            tracking_data.tracking_state = false;
            std::cout << "Error getting ZED data." << std::endl;
        }

        auto loop_dur = get_time_micro() - last_run;
        if (loop_dur > loop_time)
        {
            auto loop_dur_in_seconds = loop_dur / 1000000.0;

            log("Zed loop over time. actual: " + std::to_string(loop_dur_in_seconds) + " s should be: " + std::to_string(loop_time/1000000.0) + " s ");
        } 
        else
        {
            std::this_thread::sleep_for(std::chrono::microseconds(loop_time-loop_dur));
        }
    }
    zed.disablePositionalTracking();
    zed.close();
}

