
#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

template<typename T>
void bindSData(py::module& m, const std::string& name) {
    py::class_<SData<T>>(m, name.c_str())
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<T>::getData)
        .def("setData", &SData<T>::setData)
        .def("waitOnStateChange", &SData<T>::waitOnStateChange)
        .def("isMemoryMapped", &SData<T>::isMemoryMapped)
        .def("getBufferIndex", &SData<T>::getBufferIndex);
        
}      

PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    // Bind SData for imuData_t
    bindSData<imuData_t>(m, "SDataIMU");
    // Bind SData for systemDesired_t
    bindSData<systemDesired_t>(m, "SDataSystemDesired");
    // Bind SData for systemActual_t
    bindSData<systemActual_t>(m, "SDataSystemActual");
    // Bind SData for trackingState_t
    bindSData<positionSystem_t>(m, "SDataPositionSystem");
    // Bind SData for driveSystemState_t
    bindSData<driveSystemState_t>(m, "SDataDriveSystemState");
    // Bind SData for camera_feed_t
    bindSData<camera_feed_t>(m, "SDataCameraFeed");

    py::class_<angles_t>(m, "angles_t")
        .def(py::init<>())
        .def_readwrite("roll", &angles_t::roll)
        .def_readwrite("pitch", &angles_t::pitch)
        .def_readwrite("yaw", &angles_t::yaw);

    py::class_<rates_t>(m, "rates_t")
        .def(py::init<>())
        .def_readwrite("gyro_roll", &rates_t::gyro_roll)
        .def_readwrite("gyro_pitch", &rates_t::gyro_pitch)
        .def_readwrite("gyro_yaw", &rates_t::gyro_yaw);

    py::class_<imuData_t>(m, "imuData_t")
        .def(py::init<>())
        .def_readwrite("angles", &imuData_t::angles)
        .def_readwrite("rates", &imuData_t::rates)
        .def_readwrite("timestamp", &imuData_t::timestamp)
        .def_readwrite("quality", &imuData_t::quality)
        .def_readwrite("error", &imuData_t::error);

    py::class_<position_t>(m, "position_t")
        .def(py::init<>())
        .def_readwrite("x", &position_t::x)
        .def_readwrite("y", &position_t::y)
        .def_readwrite("z", &position_t::z);

    py::class_<systemActual_t>(m, "systemActual_t")
        .def(py::init<>())
        .def_readwrite("state", &systemActual_t::state);

    py::class_<controllerSettings_t>(m, "controllerSettings_t")
        .def(py::init<>())
        .def_readwrite("pitch_p", &controllerSettings_t::pitch_p)
        .def_readwrite("pitch_i", &controllerSettings_t::pitch_i)
        .def_readwrite("pitch_d", &controllerSettings_t::pitch_d)
        .def_readwrite("velocity_p", &controllerSettings_t::velocity_p)
        .def_readwrite("velocity_i", &controllerSettings_t::velocity_i)
        .def_readwrite("velocity_d", &controllerSettings_t::velocity_d)
        .def_readwrite("yaw_rate_p", &controllerSettings_t::yaw_rate_p)
        .def_readwrite("yaw_rate_i", &controllerSettings_t::yaw_rate_i)
        .def_readwrite("yaw_rate_d", &controllerSettings_t::yaw_rate_d)
        .def_readwrite("pitch_zero", &controllerSettings_t::pitch_zero)
        .def_readwrite("yaw_p", &controllerSettings_t::yaw_p)
        .def_readwrite("yaw_i", &controllerSettings_t::yaw_i)
        .def_readwrite("yaw_d", &controllerSettings_t::yaw_d)
        .def_readwrite("positon_p", &controllerSettings_t::positon_p)
        .def_readwrite("positon_i", &controllerSettings_t::positon_i)
        .def_readwrite("positon_d", &controllerSettings_t::positon_d)
        .def_readwrite("dead_zone_pos", &controllerSettings_t::dead_zone_pos)
        .def_readwrite("dead_zone_yaw", &controllerSettings_t::dead_zone_yaw)
        .def_readwrite("dead_zone_vel", &controllerSettings_t::dead_zone_vel)
        .def_readwrite("dead_zone_pitch", &controllerSettings_t::dead_zone_pitch)
        .def_readwrite("dead_zone_yaw_rate", &controllerSettings_t::dead_zone_yaw_rate);

    py::class_<JoystickState>(m, "JoystickState")
        .def(py::init<>())
        .def_readwrite("x", &JoystickState::x)
        .def_readwrite("y", &JoystickState::y)
        .def_readwrite("time", &JoystickState::time);


    py::class_<camera_feed_t>(m, "camera_feed_t", pybind11::buffer_protocol())
        .def(py::init<>())
        .def("dump_data", &camera_feed_t::dump_data)
        .def("getPx", &camera_feed_t::getPx)    
        .def_buffer([](camera_feed_t& im) -> pybind11::buffer_info {
            return pybind11::buffer_info(
                // Pointer to buffer
                im.frame,
                // Size of one scalar
                sizeof(unsigned char),
                // Python struct-style format descriptor
                pybind11::format_descriptor<unsigned char>::format(),
                // Number of dimensions
                3,
                // Buffer dimensions
                { im.rows, im.cols, im.channels },
                // Strides (in bytes) for each index
                {
                    sizeof(unsigned char) * im.channels * im.cols,
                    sizeof(unsigned char) * im.channels,
                    sizeof(unsigned char)
                }
            );
        });

    py::class_<DriveState>(m, "DriveState")
        .def(py::init<>())
        .def_readwrite("velocity", &DriveState::velocity)
        .def_readwrite("position", &DriveState::position)
        .def_readwrite("vBusVoltage", &DriveState::vBusVoltage)
        .def_readwrite("state", &DriveState::state)
        .def_readwrite("error", &DriveState::error);

    py::class_<positionSystem_t>(m, "positionSystem_t")
        .def(py::init<>())
        .def_readwrite("status", &positionSystem_t::status)
        .def_readwrite("position", &positionSystem_t::position)
        .def_readwrite("timestamp", &positionSystem_t::timestamp);

    py::class_<driveSystemState_t>(m, "driveSystemState_t")
        .def(py::init<>())
        .def("getAxis", &driveSystemState_t::getAxis);

    py::class_<systemDesired_t>(m, "systemDesired_t")
        .def(py::init<>())
        .def_readwrite("state", &systemDesired_t::state)
        .def_readwrite("position", &systemDesired_t::position)
        .def_readwrite("angles", &systemDesired_t::angles)
        .def_readwrite("rates", &systemDesired_t::rates)
        .def_readwrite("leftShoulder", &systemDesired_t::leftShoulder)
        .def_readwrite("rightShoulder", &systemDesired_t::rightShoulder);
    
};

