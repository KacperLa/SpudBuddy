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
       
    py::class_<sytemState_t>(m, "sytemState_t")
        .def(py::init<>())
        .def_readwrite("actual", &sytemState_t::actual)
        .def_readwrite("driveSystem", &sytemState_t::driveSystem)
        .def_readwrite("controller_settings", &sytemState_t::controller_settings);
};
