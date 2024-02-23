#!./env/bin/python3.10

import argparse
import os
import sys
import webbrowser
from mmap_functions import *

import numpy as np
import matplotlib.pyplot as plt

from pydrake.common.value import Value

from pydrake.geometry import DrakeVisualizer, Meshcat, MeshcatVisualizer
from pydrake.math import RigidTransform, RollPitchYaw, RotationMatrix
from pydrake.multibody.inverse_kinematics import (
    DifferentialInverseKinematicsIntegrator,
    DifferentialInverseKinematicsParameters)
from pydrake.multibody.plant import MultibodyPlant
from pydrake.systems.analysis import Simulator
from pydrake.systems.framework import DiagramBuilder, LeafSystem

from pydrake.all import (
    AddMultibodyPlantSceneGraph,
    Parser, RigidTransform, SpatialVelocity, RotationMatrix,
    AffineSystem, LogVectorOutput, CoulombFriction, HalfSpace, RgbdSensor,
    AbstractValue, BasicVector, ConstantVectorSource, RollPitchYaw)


# actual state struct
actual_state_format_mapping = {
    'x': 'f',
    'y': 'f',
    'z': 'f',
    'tracking_state': 'i',
    'angles_x': 'd',
    'angles_y': 'd',
    'angles_z': 'd',
    'rates_x': 'd',
    'rates_y': 'd',
    'rates_z': 'd',
    'timestamp': 'q',
    'quality': 'i',
    'error': 'i'
}

actual_state_struct = {
    'x': 0.0,
    'y': 0.0,
    'z': 0.0,
    'tracking_state': 0,
    'angles_x': 0.0,
    'angles_y': 0.0,
    'angles_z': 0.0,
    'rates_x': 0.0,
    'rates_y': 0.0,
    'rates_z': 0.0,
    'timestamp': 0,
    'quality': 0,
    'error': 0
}

MAP_NAME_ACTUAL   = "/tmp/sim_state";
SEMAPHORE_NAME_ACTUAL  = "/sim_state_sem";

actual_state = ThreadSafeStruct(actual_state_struct) 
actual_state_mmap = mmap_writer(SEMAPHORE_NAME_ACTUAL, MAP_NAME_ACTUAL, actual_state_format_mapping)   


def AddGround(plant):
    """
    Add a flat ground with friction
    """

    # Constants
    transparent_color = np.array([0.5,0.5,0.5,0])
    nontransparent_color = np.array([0.5,0.5,0.5,0.1])

    p_GroundOrigin = [0, 0.0, 0.0]
    R_GroundOrigin = RotationMatrix.MakeXRotation(0.0)
    X_GroundOrigin = RigidTransform(R_GroundOrigin,p_GroundOrigin)

    # Set Up Ground on Plant
    surface_friction = CoulombFriction(
            static_friction = 0.7,
            dynamic_friction = 0.5)
    plant.RegisterCollisionGeometry(
            plant.world_body(),
            X_GroundOrigin,
            HalfSpace(),
            "ground_collision",
            surface_friction)
    plant.RegisterVisualGeometry(
            plant.world_body(),
            X_GroundOrigin,
            HalfSpace(),
            "ground_visual",
            nontransparent_color)  # transparent

class BaseLinkStateExtractor(LeafSystem):
    def __init__(self, plant):
        LeafSystem.__init__(self)
        self.plant = plant
        self.body = plant.GetBodyByName("base_link")

        # Define the input and output ports
        self.DeclareVectorInputPort("full_state", BasicVector(plant.num_multibody_states()))
        self.DeclareVectorOutputPort("base_link_orientation", BasicVector(3), self.CalcBaseLinkOrientation)

    def CalcBaseLinkOrientation(self, context, output):
        full_state = self.get_input_port().Eval(context)
        plant_context = self.plant.CreateDefaultContext()
        self.plant.SetPositionsAndVelocities(plant_context, full_state)
        
        # Get the pose of the body in the world frame
        body_pose = self.plant.EvalBodyPoseInWorld(plant_context, self.body)
        body_angular_velocity = self.plant.EvalBodySpatialVelocityInWorld(plant_context, self.body).rotational();
        
        # Extract Euler angles (roll, pitch, yaw)
        rpy = RollPitchYaw(body_pose.rotation()).vector()
        xyz = body_pose.translation()
        actual_state.set('x', xyz[0])
        actual_state.set('y', xyz[1])
        actual_state.set('z', xyz[2])
        actual_state.set('angles_x', rpy[0])
        actual_state.set('angles_y', rpy[1]) 
        actual_state.set('angles_z', rpy[2])
        actual_state.set('rates_x', body_angular_velocity[0])
        actual_state.set('rates_y', body_angular_velocity[1])
        actual_state.set('rates_z', body_angular_velocity[2])

        actual_state_mmap.write(actual_state.get_all())  
        output.SetFromVector(rpy)

class RonController(LeafSystem):
    def __init__(self, Kp, Ki, Kd, setpoint):
        LeafSystem.__init__(self)
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
        self.setpoint = setpoint
        self.integral = 0.0  # Integral error
        self.prev_error = 0.0  #Previous error
        self.prev_time = 0.0  # Previous time

        # Define the input and output ports
        self.DeclareVectorInputPort("state", BasicVector(3))
        self.DeclareVectorOutputPort("control", BasicVector(2), self.CalculateControl)

    def CalculateControl(self, context, output):
        state = self.get_input_port().Eval(context)
        error = self.setpoint - state[0]  # Assuming state[0] is the position

        # Integral and derivative calculations
        time_delta = context.get_time() - self.prev_time
        if time_delta == 0:
            derivative = 0.0
        else:
            derivative = (error - self.prev_error) / (time_delta)
        self.prev_error = error
        self.prev_time = context.get_time()
        # PID control
        control = self.Kp * error + float(self.Kd) * float(derivative)
        # print(error, control)

        output.SetAtIndex(0, control)
        output.SetAtIndex(1, -1.0*control)

def main():
    # Variables to store simulation data
    times = []
    states = []

    builder = DiagramBuilder()

    plant, scene_graph = AddMultibodyPlantSceneGraph(builder, 0.0001)

    # Load the robot model
    file_path = 'RON/urdf/RON.urdf'  # Replace with the path to your URDF file
    parser = Parser(plant)

    parser.AddModels(file_path)
    AddGround(plant)
    plant.Finalize()

    query_port = scene_graph.get_query_output_port()

    DrakeVisualizer.AddToBuilder(builder, query_port)
    meshcat = Meshcat()
    meshcat_visualizer = MeshcatVisualizer.AddToBuilder(
        builder=builder,
        query_object_port=query_port,
        meshcat=meshcat)

    # add a imu sensor
    # imu = builder.AddSystem(RgbdSensor(parent_id=plant.GetBodyByName("base_link").index(), X_PB=RigidTransform(), show_window=False))

    state_extractor = builder.AddSystem(BaseLinkStateExtractor(plant))
    builder.Connect(plant.get_state_output_port(), state_extractor.get_input_port())

    # PID Controller
    # Set PID gains
    Kp = 5.0  # Proportional gain
    Ki = 0.0   # Integral gain
    Kd = 1.0   # Derivative gain
    setpoint = 0.2  # Setpoint

    # Create a PID controller and connect it
    pid_controller = builder.AddSystem(RonController(Kp, Ki, Kd, setpoint))
    builder.Connect(state_extractor.get_output_port(), pid_controller.get_input_port())
    builder.Connect(pid_controller.get_output_port(), plant.get_actuation_input_port())


    diagram = builder.Build()
    simulator = Simulator(diagram)

    context = simulator.get_mutable_context()
    plant_context = plant.GetMyMutableContextFromRoot(context)

    base_frame = plant.GetBodyByName("base_link")  # Replace "base_link" with your robot's base link name
    initial_pose = RigidTransform(RotationMatrix.Identity(), [0, 0, 5])  # Replace [0, 0, 1] with desired [x, y, z] position
    plant.SetFreeBodyPose(plant_context, base_frame, initial_pose)

    simulator.set_target_realtime_rate(1)

    # simulator.AdvanceTo(5.0)
    while simulator.get_context().get_time() < 5.0:
        simulator.AdvanceTo(simulator.get_context().get_time() + 0.01)
        times.append(simulator.get_context().get_time())
        plant_state = plant.EvalBodyPoseInWorld(plant_context, plant.GetBodyByName("base_link"))
        # Extract Euler angles (roll, pitch, yaw)
        rpy = RollPitchYaw(plant_state.rotation()).vector()
        state = rpy[0]
        states.append(state)

    # Plot the results using Matplotlib
    plt.figure()
    plt.plot(times, states)
    plt.xlabel('Time (s)')
    plt.ylabel('State')
    plt.title('System State Over Time')
    plt.grid(True)
    # plt.show()

if __name__ == '__main__':
    main()