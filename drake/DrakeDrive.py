#!./env/bin/python3.10


import json
import math
import struct
import copy
import numpy as np
import time
import threading

class AxisState_t:
    AXIS_STATE_UNDEFINED = 0                   #<! will fall through to idle
    AXIS_STATE_IDLE = 1                        #<! disable PWM and do nothing
    AXIS_STATE_STARTUP_SEQUENCE = 2            #<! the actual sequence is defined by the config.startup_... flags
    AXIS_STATE_FULL_CALIBRATION_SEQUENCE = 3   #<! run all calibration procedures, then idle
    AXIS_STATE_MOTOR_CALIBRATION = 4           #<! run motor calibration
    AXIS_STATE_SENSORLESS_CONTROL = 5          #<! run sensorless control
    AXIS_STATE_ENCODER_INDEX_SEARCH = 6        #<! run encoder index search
    AXIS_STATE_ENCODER_OFFSET_CALIBRATION = 7  #<! run encoder offset calibration
    AXIS_STATE_CLOSED_LOOP_CONTROL = 8         #<! run closed loop control

class ControlMode_t:
    VOLTAGE_CONTROL = 0
    TORQUE_CONTROL = 1
    VELOCITY_CONTROL = 2
    POSITION_CONTROL = 3
	
class InputMode_t:
    INACTIVE = 0
    PASSTHROUGH = 1
    VEL_RAMP = 2
    POS_FILTER = 3
    MIX_CHANNELS = 4
    TRAP_TRAJ = 5
    TORQUE_RAMP = 6
    MIRROR = 7
    TUNING = 8

class CommandID:
    CMD_ID_CANOPEN_NMT_MESSAGE = 0x000
    CMD_ID_ODRIVE_HEARTBEAT_MESSAGE = 0x001
    CMD_ID_ODRIVE_ESTOP_MESSAGE = 0x002
    CMD_ID_GET_MOTOR_ERROR = 0x003
    CMD_ID_GET_ENCODER_ERROR = 0x004
    CMD_ID_GET_SENSORLESS_ERROR = 0x005
    CMD_ID_SET_AXIS_NODE_ID = 0x006
    CMD_ID_SET_AXIS_REQUESTED_STATE = 0x007
    CMD_ID_SET_AXIS_STARTUP_CONFIG = 0x008
    CMD_ID_GET_ENCODER_ESTIMATES = 0x009
    CMD_ID_GET_ENCODER_COUNT = 0x00A
    CMD_ID_SET_CONTROLLER_MODES = 0x00B
    CMD_ID_SET_INPUT_POS = 0x00C
    CMD_ID_SET_INPUT_VEL = 0x00D
    CMD_ID_SET_INPUT_TORQUE = 0x00E
    CMD_ID_SET_LIMITS = 0x00F
    CMD_ID_START_ANTICOGGING = 0x010
    CMD_ID_SET_TRAJ_VEL_LIMIT = 0x011
    CMD_ID_SET_TRAJ_ACCEL_LIMITS = 0x012
    CMD_ID_SET_TRAJ_INERTIA = 0x013
    CMD_ID_GET_IQ = 0x014
    CMD_ID_GET_SENSORLESS_ESTIMATES = 0x015
    CMD_ID_REBOOT_ODRIVE = 0x016
    CMD_ID_GET_VBUS_VOLTAGE = 0x017
    CMD_ID_CLEAR_ERRORS = 0x018
    CMD_ID_SET_LINEAR_COUNT = 0x019
    CMD_ID_SET_POS_GAIN = 0x01A
    CMD_ID_SET_VEL_GAINS = 0x01B
    CMD_ID_GET_ADC_VOLTAGE = 0x01C
    CMD_ID_GET_CONTROLLER_ERROR = 0x01D
    CMD_ID_CANOPEN_HEARTBEAT_MESSAGE = 0x700
    
import can
can.rc['interface'] = 'socketcan'
can.rc['channel'] = 'can0'
can.rc['bitrate'] = 1000000
from can import Bus
bus = Bus()
bus.receive_own_messages = False

class FilteredCanReader(can.BufferedReader):
    def __init__(self):
        can.BufferedReader.__init__(self)
        self.func_map = {}
        self.domain = None
        
    def register_callback(self, address):
        def wrapper(function):
            self.func_map[str(int(address))] = function
            return function
        return wrapper

    def on_message_received(self, data=None):
        bin_id = bin(data.arbitration_id)
        print(bin_id)
        if len(bin_id) > 7:
            id = str(int(bin_id[7:],2))
            func = self.func_map.get(id, None)
            if func is None:
                print("No function registered against")
                print(data)
            else:
                return func(self.domain, data)
        else:
            print("Can Address to short")
            print(data)
        
    
fbr = FilteredCanReader()
notifier = can.Notifier(bus, [fbr])

import math
import os
import json

while True:
    time.sleep(1)

# class Module:
#     """
#     Class responsible for handling the function of the ODrive
#     """
#     def __init__(self, bus, name, config, enabled):
#         """
#         Initialise ODrive using given config JSON object
#         :param config: Properties specific to given module
#         :param enabled:
#         """
#         self.bus = bus
#         self.enabled = enabled
#         self.state = [0, 0]

#         # ODrive
#         self.odrive_name = name
#         self.odrive = None
#         print(config)
#         self.odrive_id_front = int(config['node_id_front'],16)
#         self.odrive_id_back = int(config['node_id_back'],16)

#         # constants
#         self.front_cprad = (2000*4.5)/(-math.pi*2)
#         self.back_cprad = (2000*4.5)/(-math.pi*2)


#         # calibration variables
#         self.back_offset = config['back_offset']
#         self.front_offset = config['front_offset']
#         self.boot_offset_front = None
#         self.boot_offset_back = None
#         self.offset_front = 0
#         self.offset_back = 0
#         self.pos_estimate_front = 0
#         self.pos_estimate_back = 0
#         # set points
#         self.back_setpoint = None
#         self.front_setpoint = None
#         self.back_n = None
#         self.front_n = None
        
#     def request_pos(self):
#         msg1 = can.Message(arbitration_id=self.odrive_id_front + 0x009, data=[],is_remote_frame=True, is_extended_id=False)
#         msg2 = can.Message(arbitration_id=self.odrive_id_back + 0x009, data=[],is_remote_frame=True, is_extended_id=False)
#         self.bus.send(msg1)
#         self.bus.send(msg2)

#     def update_pos(self, data):
#         pos_estimate = struct.unpack('f', data.data[0:4])[0]
#         if data.arbitration_id == self.odrive_id_front + 0x009 and data.dlc == 8:
#             self.pos_estimate_front = pos_estimate
#         elif data.arbitration_id == self.odrive_id_back + 0x009 and data.dlc == 8:
#             self.pos_estimate_back = pos_estimate
#         #print("front: ", self.pos_estimate_front, "back: ", self.pos_estimate_back)
#         self.front_n = (self.pos_estimate_front - self.front_offset + ((math.pi/2)*self.front_cprad)) / self.front_cprad
#         self.back_n =  (self.pos_estimate_back - self.back_offset - ((math.pi/2)*self.back_cprad)) / self.back_cprad
#         #print("front_deg_real: ", math.degrees(front_n), "bac_deg_real: ", math.degrees(back_n))
#         #print("encoder pos: ", struct.unpack('f', data.data[0:4]))
#         #print("encoder vel: ", struct.unpack('f', data.data[4:8])) 

#     def update_state(self, data):
#         data_unpacked = struct.unpack('II', data.data)[1]
#         if data.arbitration_id == self.odrive_id_front + 0x001 and data.dlc == 8:
#             self.state[0] = data_unpacked
#         elif data.arbitration_id == self.odrive_id_back + 0x001 and data.dlc == 8:
#             self.state[1] = data_unpacked

#     def start(self):
#         print('starting module')
#         #await self.home()

#     def scale_setpoint(self, angle, gamma):
#         front_rad = angle+gamma
#         back_rad = angle-gamma
#         count_front_offset, count_back_offset = ((math.pi/2)*self.front_cprad), ((math.pi/2)*self.back_cprad)
#         count_front = (front_rad * self.front_cprad)- count_front_offset + self.front_offset
#         count_back = (back_rad * self.back_cprad) + count_back_offset + self.back_offset
#         #print("angle: ", angle, "gomma: ", gamma)
#         #print("count_front: ", count_front, "count_back: ", count_back)
#         front_n = (count_front - self.front_offset + ((math.pi/2)*self.front_cprad)) / self.front_cprad
#         back_n =  (count_back - self.back_offset - ((math.pi/2)*self.back_cprad)) / self.back_cprad
#         #print("front_deg_d: ", math.degrees(front_n), "bac_deg_d: ", math.degrees(back_n))
        
#         return count_front, count_back

#     def enable(self):
#         if self.enabled:
#             msg_1 = can.Message(arbitration_id=self.odrive_id_front + 0x007, data=struct.pack('I', 8),is_remote_frame=False, is_extended_id=False)
#             msg_2 = can.Message(arbitration_id=self.odrive_id_back + 0x007, data=struct.pack('I', 8),is_remote_frame=False, is_extended_id=False)
#             self.bus.send(msg_1)
#             self.bus.send(msg_2)

#     def disable(self):
#        if self.enabled:
#            msg_1 = can.Message(arbitration_id=self.odrive_id_front + 0x007, data=struct.pack('I', 1),is_remote_frame=False, is_extended_id=False)
#            msg_2 = can.Message(arbitration_id=self.odrive_id_back + 0x007, data=struct.pack('I', 1),is_remote_frame=False, is_extended_id=False)
#            self.bus.send(msg_1)
#            self.bus.send(msg_2)
           
#     def calibrate_encoders(self):
#        if self.enabled:
#            msg_1 = can.Message(arbitration_id=self.odrive_id_front + 0x007, data=struct.pack('I', 7),is_remote_frame=False, is_extended_id=False)
#            msg_2 = can.Message(arbitration_id=self.odrive_id_back + 0x007, data=struct.pack('I', 7),is_remote_frame=False, is_extended_id=False)
#            self.bus.send(msg_1)
#            self.bus.send(msg_2)
           
#     def reboot(self):
#        if self.enabled:
#            msg_1 = can.Message(arbitration_id=self.odrive_id_front + 0x016, is_remote_frame=False, is_extended_id=False)
#            self.bus.send(msg_1)
#            print("rebooting")

#     def set_commands(self, angle, gamma):
#         self.angle_setpoint = angle  # scaled mm
#         self.gamma_setpoint = gamma # scaled mm

#     def run(self):
#         if self.enabled:
#             if not math.isnan(self.angle_setpoint) and not math.isnan(self.gamma_setpoint):
#                 count_front, count_back = self.scale_setpoint(self.angle_setpoint, self.gamma_setpoint)

#                 msg_front = can.Message(arbitration_id=self.odrive_id_front + 0x00c, data=struct.pack('ihh', int(count_front), 0, 0),is_remote_frame=False, is_extended_id=False)
#                 msg_back = can.Message(arbitration_id=self.odrive_id_back + 0x00c, data=struct.pack('ihh', int(count_back), 0, 0),is_remote_frame=False, is_extended_id=False)
#                 self.bus.send(msg_front)
#                 self.bus.send(msg_back)
#             else:
#                 pass

#     def home(self):
#         print('starting home')
#         if self.enabled:
#             self.front_offset = self.pos_estimate_front
#             self.back_offset = self.pos_estimate_back
#             print("back offset: ", self.back_offset, "front offset: ", self.front_offset)            
#         print('ending home')

#     def request_bus_voltage(self):
#         """
#         Send bus voltage request to odrive
#         :return: return bus voltage
#         """
#         msg = can.Message(arbitration_id=self.odrive_id_front + 0x017, data=[],is_remote_frame=True, is_extended_id=False)
#         bus.send(msg)

   


# class CanBusNode():
#     def __init__(self):
        
#         self.dolly = Dolly(bus, config['dolly'])
#         fbr.domain = self
    
#     def request_pos(self):
#         self.dolly.request_pos()
#         msg_out = Twist()
#         if self.dolly.modules['back'].back_n is not None and self.dolly.modules['back'].front_n is not None:
#             msg_out.linear = Vector3(x=0.0 , y=0.0, z=0.0)
#             msg_out.angular = Vector3(x=self.dolly.modules['back'].back_n, y=self.dolly.modules['back'].front_n, z=0.0)
#             self.pose_publisher.publish(msg_out)
    
#     @fbr.register_callback(CommandID.CMD_ID_GET_VBUS_VOLTAGE)
#     def publish_bus_voltage(self, data):
# 	    actual_data = actual_state_mmap.read(actual_state_struct)
#         print("voltage_callback")
#         msg = createVolatageMsg(actual_data)
#         can.Message(arbitration_id=self.odrive_id_front + 0x017,
#                     data=[], 
#                     is_remote_frame=False,
#                     is_extended_id=False)

    
#     @fbr.register_callback(0x009)
#     def update_pos_estimate(self, data):
#         self.dolly.update_pos(data)
        
#     @fbr.register_callback(0x001)
#     def watch_dog(self, data):
#         self.dolly.update_state(data)

        
#     def calibrate_encoders_callback(self, request, response):
#         self.dolly.calibrate_encoders()
#         print("Send Dolly calibrate cmd")
#         response.success = True
#         response.message = "calibrating"
#         return response
    
#     def enable_dolly_callback(self, request, response):
#         start_time = time.time()
#         print("Send Dolly enable cmd")
#         self.dolly.enable()
#         done = False
#         while not done:
#             if not False in np.where(self.dolly.check_state() == 8, True, False):
#                 done = True
#                 response.success = True
#                 response.message = "Success"
#             elif time.time() - start_time > 2:
#                 done = True
#                 print("Failed Enable timeout")
#                 response.success = False
#                 response.message = "Failed"
#         return response
    
#     def disable_dolly_callback(self, request, response):
#         start_time = time.time()
#         self.dolly.disable()
#         print("Send Dolly disable cmd")
#         done = False
#         while not done:
#             if not False in np.where(self.dolly.check_state() == 1, True, False):
#                 done = True
#                 response.success = True
#                 response.message = "Success"
#             elif time.time() - start_time > 2:
#                 done = True
#                 print("Failed Enable timeout")
#                 response.success = False
#                 response.message = "Failed"
#         return response
    
#     def home_dolly_callback(self, request, response):
#         self.dolly.home()
#         print("Send Doggo home cmd")
#         response.success = True
#         response.message = "Homed"
#         return response
    
#     def reboot_dolly_callback(self, request, response):
#         self.dolly.reboot()
#         print("Send Doggo reboot cmd")
#         response.success = True
#         response.message = "rebooting"
#         return response




#     # Member function that takes a dict and creates a can_frame
#     def createHeartbeatMsg(data):
#         format_string = 'IBBBB'
     
#         # return bin string to long
#         controllerString = '00000001' if data['controllerSet'] else '00000000'
#         controllerFlag = int(controllerString, 2)   
#         trajectoryDone = '10000000' if data["trajectorySet"] else '00000000'
#         trajectoryDone = int(trajectoryDone, 2)
#         trajectoryDone_controllerFlag = trajectoryDone | controllerFlag
        
#         data = [data["axisError"], 
#                 data["currentState"], 
#                 data["motorFlag"], 
#                 data["encoderFlag"], 
#                 trajectoryDone_controllerFlag]

#         return struct.pack(format_string, *data)

# 	def createEncoderEstimatesMsg(data):
#         format_string = 'ff'
#         data = [data['posEstimate'], data['velEstimate']]
#         return struct.pack(format_string, *data))


# # actual state struct
# actual_state_format_mapping = {
#     'posEstimate': 'f',
#     'velEstimate': 'f',
#     'z': 'f',
#     'tracking_state': 'i',
#     'angles_x': 'd',
#     'angles_y': 'd',
#     'angles_z': 'd',
#     'rates_x': 'd',
#     'rates_y': 'd',
#     'rates_z': 'd',
#     'timestamp': 'q',
#     'quality': 'i',
#     'error': 'i'
# }

# actual_state_struct = {
#     'x': 0.0,
#     'y': 0.0,
#     'z': 0.0,
#     'tracking_state': 0,
#     'angles_x': 0.0,
#     'angles_y': 0.0,
#     'angles_z': 0.0,
#     'rates_x': 0.0,
#     'rates_y': 0.0,
#     'rates_z': 0.0,
#     'timestamp': 0,
#     'quality': 0,
#     'error': 0
# }

# MAP_NAME_ACTUAL   = "/tmp/sim_state";
# SEMAPHORE_NAME_ACTUAL  = "/sim_state_sem";

# actual_state = ThreadSafeStruct(actual_state_struct) 
# actual_state_mmap = mmap_writer(SEMAPHORE_NAME_ACTUAL, MAP_NAME_ACTUAL, actual_state_format_mapping)   
#         msg = can.Message(arbitration_id=self.odrive_id_front + 0x017, data=[],is_remote_frame=True, is_extended_id=False)


# def main(args=None):
#     # robot state thread safe structs
#     status = Status()
#     cmd    = Command() 

#     bus = CanBusNode()
#     mem = SharedMemory()   

#     # start a thread reading from shared memory
#     t1 = threading.Thread(target=mem.process, args=()) 

#     # start a thread to process reqs from can
#     t2 = threading.Thread(target=bus.recv, args=())


# if __name__ == '__main__':
#     main()