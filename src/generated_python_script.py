
#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    py::class_<SData<sytemState_t>>(m, "SData")
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<sytemState_t>::getData)
        .def("setData", &SData<sytemState_t>::setData);
    
    py::class_<IMUState>(m, "IMUState")
    .def(py::init<>())
    .def_readwrite("angles", &IMUState::angles)
    .def_readwrite("rates", &IMUState::rates)
    .def_readwrite("timestamp", &IMUState::timestamp)
    .def_readwrite("quality", &IMUState::quality)
    .def_readwrite("error", &IMUState::error);


#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

// py bindings for systemState_t


PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    py::class_<SData<sytemState_t>>(m, "SData")
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<sytemState_t>::getData)
        .def("setData", &SData<sytemState_t>::setData);
    
    py::class_<position_t>(m, "position_t")
    .def(py::init<>())
    .def_readwrite("x", &position_t::x)
    .def_readwrite("y", &position_t::y)
    .def_readwrite("z", &position_t::z);


#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

// py bindings for systemState_t


PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    py::class_<SData<sytemState_t>>(m, "SData")
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<sytemState_t>::getData)
        .def("setData", &SData<sytemState_t>::setData);
    
    py::class_<robot_state_t>(m, "robot_state_t")
    .def(py::init<>())
    .def_readwrite("position", &robot_state_t::position)
    .def_readwrite("positionDeadReckoning", &robot_state_t::positionDeadReckoning)
    .def_readwrite("positionSlam", &robot_state_t::positionSlam)
    .def_readwrite("positionStatus", &robot_state_t::positionStatus)
    .def_readwrite("angles", &robot_state_t::angles)
    .def_readwrite("rates", &robot_state_t::rates)
    .def_readwrite("velocity", &robot_state_t::velocity)
    .def_readwrite("leftVelocity", &robot_state_t::leftVelocity)
    .def_readwrite("rightVelocity", &robot_state_t::rightVelocity)
    .def_readwrite("state", &robot_state_t::state);


#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

// py bindings for systemState_t


PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    py::class_<SData<sytemState_t>>(m, "SData")
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<sytemState_t>::getData)
        .def("setData", &SData<sytemState_t>::setData);
    
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
    .def_readwrite("dead_zone", &controllerSettings_t::dead_zone);


#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

// py bindings for systemState_t


PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    py::class_<SData<sytemState_t>>(m, "SData")
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<sytemState_t>::getData)
        .def("setData", &SData<sytemState_t>::setData);
    
    py::class_<JoystickState>(m, "JoystickState")
    .def(py::init<>())
    .def_readwrite("x", &JoystickState::x)
    .def_readwrite("y", &JoystickState::y)
    .def_readwrite("time", &JoystickState::time);


#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

// py bindings for systemState_t


PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    py::class_<SData<sytemState_t>>(m, "SData")
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<sytemState_t>::getData)
        .def("setData", &SData<sytemState_t>::setData);
    
    py::class_<DriveState>(m, "DriveState")
    .def(py::init<>())
    .def_readwrite("velocity", &DriveState::velocity)
    .def_readwrite("position", &DriveState::position)
    .def_readwrite("vBusVoltage", &DriveState::vBusVoltage)
    .def_readwrite("state", &DriveState::state)
    .def_readwrite("error", &DriveState::error);


#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

// py bindings for systemState_t


PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    py::class_<SData<sytemState_t>>(m, "SData")
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<sytemState_t>::getData)
        .def("setData", &SData<sytemState_t>::setData);
    
    py::class_<driveSystemState>(m, "driveSystemState")
    .def(py::init<>())
    .def_readwrite("axis_0", &driveSystemState::axis_0)
    .def_readwrite("axis_1", &driveSystemState::axis_1);


#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

// py bindings for systemState_t


PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    py::class_<SData<sytemState_t>>(m, "SData")
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<sytemState_t>::getData)
        .def("setData", &SData<sytemState_t>::setData);
    
    py::class_<sytemState_t>(m, "sytemState_t")
    .def(py::init<>())
    .def_readwrite("actual", &sytemState_t::actual)
    .def_readwrite("driveSystem", &sytemState_t::driveSystem)
    .def_readwrite("controller_settings", &sytemState_t::controller_settings);


#include <sdata.h>
#include <shared_structs.h>
#include "libraries/pybind11/include/pybind11/pybind11.h"

namespace py = pybind11;    

// py bindings for systemState_t


PYBIND11_MODULE(SDataLib, m) {

    m.doc() = "pybind11 sdata plugin"; // optional module docstring

    py::class_<SData<sytemState_t>>(m, "SData")
        .def(py::init<const std::string&, bool>())
        .def("getData", &SData<sytemState_t>::getData)
        .def("setData", &SData<sytemState_t>::setData);
    
    py::class_<systemDesired_t>(m, "systemDesired_t")
    .def(py::init<>())
    .def_readwrite("state", &systemDesired_t::state)
    .def_readwrite("joystick", &systemDesired_t::joystick)
    .def_readwrite("position", &systemDesired_t::position);

