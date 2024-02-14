#include "ZEDReader.h"
#include <sys/resource.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include "common.h"

ZEDReader::ZEDReader(const std::string& areaFile, const std::string name, Log* logger) :
  ronThread(name, logger),
  shared_tracking_state(logger, shared_tracking_state_file,  true)
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

    // SpatialMappingParameters mapping_parameters;
    // mapping_parameters.save_texture = true;  // Scene texture will be recorded
    // returned_state = zed.enableSpatialMapping(mapping_parameters);
    // if (returned_state != ERROR_CODE::SUCCESS)
    // {
    //     log("Enabling spatial mapping failed");
    //     zed.close();
    //     return false;
    // }

    return true;
}

void ZEDReader::stop() {
  running = false;
}

void ZEDReader::getIMUData(imuData_t& data)
{
    static SensorsData sensors_data;
    if (zed.getSensorsData(sensors_data, TIME_REFERENCE::CURRENT ) == sl::ERROR_CODE::SUCCESS)
    {
        data.angles.yaw       = -sensors_data.imu.pose.getEulerAngles(true)[0];
        data.angles.pitch     = -sensors_data.imu.pose.getEulerAngles(true)[1];
        data.angles.roll      =  sensors_data.imu.pose.getEulerAngles(true)[2];
        data.rates.gyro_yaw   = -sensors_data.imu.angular_velocity[0];
        data.rates.gyro_pitch = -sensors_data.imu.angular_velocity[1];
        data.rates.gyro_roll  =  sensors_data.imu.angular_velocity[2];
        data.timestamp        =  sensors_data.imu.timestamp.getNanoseconds();
        data.error            =  false; 
        // log("yaw: " + std::to_string(data.angles.yaw) + " pitch: " + std::to_string(data.angles.pitch) + " roll: " + std::to_string(data.angles.roll));
    }
    else
    {
        data.error = true;
    }
}

void ZEDReader::saveMesh()
{
    Mesh mesh; 
    zed.extractWholeSpatialMap(mesh);
    mesh.applyTexture();
    mesh.filter(MeshFilterParameters::MESH_FILTER::LOW); // Filter the mesh (remove unnecessary vertices and faces)
    mesh.save("server/static/mesh.obj");
}

void ZEDReader::loop() {
    SensorsData sensors_data;
    Pose camera_path;
    POSITIONAL_TRACKING_STATE tracking_state;

    // set error to true untill camera is open
    trackingState_t tracking_data;
    tracking_data.is_tracking = false;
    shared_tracking_state.setData(tracking_data);

    // Open the ZED device
    if (open() != 1) {
        std::cerr << "Error opening ZED." << std::endl;
        return;
    }

    while (running) {
        auto last_run = get_time_nano();
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
                tracking_data.is_tracking = true;
            } else {
                tracking_data.is_tracking = false;
            }
        }  
        else
        {
            // watch dog set error is no new readingin a while
            // std::cerr << "Error getting ZED data." << std::endl;
            tracking_data.is_tracking = false;
            std::cout << "Error getting ZED data." << std::endl;
        }
        
        shared_tracking_state.setData(tracking_data);

        auto loop_dur = get_time_nano() - last_run;
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
    // zed.disableSpatialMapping();
    zed.close();
}

