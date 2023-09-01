#!venv/bin/python

import json
import numpy as np
import time
import cv2
import depthai as dai
import zmq
from pathlib import Path

context = zmq.Context()

# Closer-in minimum depth, disparity range is doubled (from 95 to 190):
extended_disparity = False
# Better accuracy for longer distance, fractional disparity 32-levels:
subpixel = True
# Better handling for occlusions:
lr_check = True


rgb_socket = context.socket(zmq.PUB)
rgb_socket.bind("tcp://*:5530")
depth_socket = context.socket(zmq.PUB)
depth_socket.bind("tcp://*:5500")



print("starting camera")
# Create pipeline
pipeline = dai.Pipeline()

# INPUT / OUTPUT
camRgb            = pipeline.create(dai.node.ColorCamera)
monoleft          = pipeline.create(dai.node.MonoCamera)
monoRight         = pipeline.create(dai.node.MonoCamera)
depth             = pipeline.create(dai.node.StereoDepth)
xoutdisparity     = pipeline.create(dai.node.XLinkOut)
xoutVideo         = pipeline.create(dai.node.XLinkOut)

# NAME STREAMS
xoutVideo      .setStreamName("video")
xoutdisparity  .setStreamName('disparity')

# PROP
# RGB
camRgb.setBoardSocket(dai.CameraBoardSocket.RGB)
camRgb.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
camRgb.setIspScale(1,2)

# MONO
monoleft.setBoardSocket(dai.CameraBoardSocket.LEFT)
monoRight.setBoardSocket(dai.CameraBoardSocket.RIGHT)
for cam in [monoleft, monoRight]: # Common config
    cam.setResolution(dai.MonoCameraProperties.SensorResolution.THE_400_P)
    #cam.setFps(20.0)

# STEREO
config = depth.initialConfig.get()
config.postProcessing.speckleFilter.enable = False
config.postProcessing.speckleFilter.speckleRange = 50
config.postProcessing.temporalFilter.enable = True
config.postProcessing.spatialFilter.enable = True
config.postProcessing.spatialFilter.holeFillingRadius = 2
config.postProcessing.spatialFilter.numIterations = 1
config.postProcessing.thresholdFilter.minRange = 400
config.postProcessing.thresholdFilter.maxRange = 200000
config.postProcessing.decimationFilter.decimationFactor = 1
depth.initialConfig.set(config)

# Properties
# RGB ENC
videoEnc = pipeline.create(dai.node.VideoEncoder)
videoEnc.setDefaultProfilePreset(1, dai.VideoEncoderProperties.Profile.MJPEG)

# DEPTH
depth.setDefaultProfilePreset(dai.node.StereoDepth.PresetMode.HIGH_ACCURACY)
# Options: MEDIAN_OFF, KERNEL_3x3, KERNEL_5x5, KERNEL_7x7 (default)
depth.initialConfig.setMedianFilter(dai.MedianFilter.KERNEL_7x7)
depth.initialConfig.setConfidenceThreshold(200)
depth.setRectifyEdgeFillColor(0) # Black, to better see the cutout
depth.setLeftRightCheck(lr_check)
depth.setExtendedDisparity(extended_disparity)
depth.setSubpixel(subpixel)
depth.initialConfig.setSubpixelFractionalBits(5)



#LINKING
camRgb    .video     .link(videoEnc.input)
monoleft  .out       .link(depth.left)
monoRight .out       .link(depth.right)
depth     .disparity .link(xoutdisparity.input)

# OUTPUT
videoEnc.bitstream.link(xoutVideo.input)
xoutVideo.input.setBlocking(False)
xoutVideo.input.setQueueSize(1)

def create_xyz(device, width, height):
    calibData = device.readCalibration()
    M_right = calibData.getCameraIntrinsics(
        dai.CameraBoardSocket.RIGHT, dai.Size2f(width, height))
    camera_matrix = np.array(M_right).reshape(3, 3)

    xs = np.linspace(0, width - 1, width, dtype=np.float32)
    ys = np.linspace(0, height - 1, height, dtype=np.float32)

    # generate grid by stacking coordinates
    base_grid = np.stack(np.meshgrid(xs, ys))  # WxHx2
    points_2d = base_grid.transpose(1, 2, 0)  # 1xHxWx2

    # unpack coordinates
    u_coord: np.array = points_2d[..., 0]
    v_coord: np.array = points_2d[..., 1]

    # unpack intrinsics
    fx: np.array = camera_matrix[0, 0]
    fy: np.array = camera_matrix[1, 1]
    cx: np.array = camera_matrix[0, 2]
    cy: np.array = camera_matrix[1, 2]

    # projective
    x_coord: np.array = (u_coord - cx) / fx
    y_coord: np.array = (v_coord - cy) / fy

    xyz = np.stack([x_coord, y_coord], axis=-1)
    return np.pad(xyz, ((0, 0), (0, 0), (0, 1)), "constant", constant_values=1.0)


# Connect to device and start pipeline
with dai.Device(pipeline) as device:
    # get intricic
    calibObj = device.readCalibration()

    width  = monoleft.getResolutionWidth()
    height = monoRight.getResolutionHeight()

    M_left   = np.array(calibObj.getCameraIntrinsics(calibObj.getStereoLeftCameraId(), width, height))
    M_right  = np.array(calibObj.getCameraIntrinsics(calibObj.getStereoRightCameraId(), width, height))
   
    R1 = np.array(calibObj.getStereoLeftRectificationRotation())
    R2 = np.array(calibObj.getStereoRightRectificationRotation())

    H_left  = np.matmul(np.matmul(M_right, R1), np.linalg.inv(M_left))
    H_right = np.matmul(np.matmul(M_right, R2), np.linalg.inv(M_right))

    baseline =  calibObj.getBaselineDistance() * 10 # mm
    focalLength = M_right[0][0]
    dispScaleFactor = baseline * focalLength

    # Output queue will be used to get the rgb frames from the output defined above
    qStill = device.getOutputQueue(name="video", maxSize=1, blocking=False)
    qDisp = device.getOutputQueue("disparity", maxSize=1, blocking=False)

    xyz = None 
    while True:
        videoIn     = qStill.get()
     
        inDisp      = qDisp.tryGet()

        if videoIn is not None:
            frame = videoIn.getData()
            # Get BGR frame from NV12 encoded video frame to show with opencv
            # Visualizing the frame on slower hosts might have overhead
            img = bytearray(frame)
            rgb_socket.send(img)

        if inDisp is not None:
            # data = inDisp.getFrame()
            # array_crop = data[200:350, :]

            # with np.errstate(divide='ignore'):
            #     norm = (dispScaleFactor / array_crop)  / 1000
            
            # masked_array = np.ma.masked_array(norm, mask=(np.isinf(norm)))

            # depthData = masked_array.mean(axis=0).tolist() 

            # # depthData = np.average(masked_array.reshape(-1, 10), axis=1).tolist()
            
            depth_frame = inDisp.getCvFrame()
            with np.errstate(divide='ignore'):
                depth_frame = (dispScaleFactor / depth_frame)
            
            if xyz is None:
                xyz = create_xyz(device, depth_frame.shape[1], depth_frame.shape[0])
            
            depth_frame = np.expand_dims(np.array(depth_frame), axis=-1)
            # To meters and reshape for rerun
            pcl = (xyz * (depth_frame) / 10.0)#.reshape(-1, 3)
            
            
            output  = np.where(np.isinf(pcl[200]), 0, pcl[200])
            json_string = {
                "frame": output.tolist()
            }
            depth_socket.send_string(json.dumps(json_string))
        
            # frameDisparity = (frameDisparityorg*disparityMultiplier).astype(np.uint8)
            # frameDisparity = cv2.applyColorMdataap(frameDisparity, cv2.COLORMAP_JET)
            # frame = cv2.imencode('.jpg', data)[1]
            # img = bytearray(frame)
            # rgb_socket.send(img)
          
          
