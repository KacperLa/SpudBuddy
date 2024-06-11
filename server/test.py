import asyncio
import json
import os
import uuid
import numpy as np
import time
import threading

from aiohttp import web
from aiortc import MediaStreamTrack, RTCRtpSender, RTCPeerConnection, RTCSessionDescription, VideoStreamTrack, RTCDataChannelParameters
from av import Packet
from fractions import Fraction

import depthai as dai
import SDataLib

ROOT = os.path.dirname(__file__)

pcs = set()

MAP_NAME_ACTUAL  = 'robot_actual'
MAP_NAME_COMMAND = 'robot_command' 
MAP_NAME_SETTINGS = 'robot_settings'
MAP_NAME_IMU  = 'robot_imu'
MAP_NAME_DRIVE_SYSTEM = 'robot_drive_system'
MAP_NAME_TRACKING = 'robot_tracking'
MAP_NAME_CAMERA = 'robot_camera'
MAP_NAME_PC = 'robot_point_cloud'
MAP_PATH = "./maps/"

depth_packet = None

def force_codec(pc, sender, forced_codec):
    kind = forced_codec.split("/")[0]
    codecs = RTCRtpSender.getCapabilities(kind).codecs
    transceiver = next(t for t in pc.getTransceivers() if t.sender == sender)
    transceiver.setCodecPreferences(
        [codec for codec in codecs if codec.mimeType == forced_codec]
    )

class RobotCommad:
    def __init__(self):
        self.command_writer = SDataLib.SDataSystemDesired(MAP_NAME_COMMAND, True)
        self.command_state  = SDataLib.systemDesired_t()
        
        # create a lock to prevent the use of setData from diferent routines
        self.lock = asyncio.Lock()

    async def setJoystick(self, data):
        async with self.lock:
            self.command_state.x = data.get(x, 0)
            self.command_state.y = data.get(y, 0)
            self.command.state.timestamp = data.get(timestamp, 0)

class RobotSettings:
    def __init__(self):
        self.writer = SDataLib.SDataSystemDesired(MAP_NAME_SETTINGS, True)
        self.state  = SDataLib.systemSettings_t()
        
        # create a lock to prevent the use of setData from diferent routines
        self.lock = asyncio.Lock()

    async def setJoystick(self, data):
        async with self.lock:
            self.command_state.x = data.get(x, 0)
            self.command_state.y = data.get(y, 0)
            self.command.state.timestamp = data.get(timestamp, 0)


class ColorVideoTransformTrack(MediaStreamTrack):
    """
    A video stream track that transforms frames from an another track.
    """

    kind = "video"

    def __init__(self):
        super().__init__()  # don't forget this!
        self.pipeline = dai.Pipeline()

        self.done = False

        # Closer-in minimum depth, disparity range is doubled (from 95 to 190):
        extended_disparity = True
        # Better accuracy for longer distance, fractional disparity 32-levels:
        subpixel = False
        # Better handling for occlusions:
        lr_check = True

        # Define sources and output
        self.camRgb = self.pipeline.create(dai.node.ColorCamera)
        self.monoLeft = self.pipeline.create(dai.node.MonoCamera)
        self.monoRight = self.pipeline.create(dai.node.MonoCamera)

        self.depth = self.pipeline.create(dai.node.StereoDepth)
        self.depthEnc = self.pipeline.create(dai.node.VideoEncoder)
        self.colorEnc = self.pipeline.create(dai.node.VideoEncoder)
        self.depthOut = self.pipeline.create(dai.node.XLinkOut)
        self.colorOut = self.pipeline.create(dai.node.XLinkOut)
        self.configIn = self.pipeline.create(dai.node.XLinkIn)

        self.depthOut.setStreamName('depth')
        self.colorOut.setStreamName('color')

        self.configIn.setStreamName('config')

        # Properties
        self.camRgb.setBoardSocket(dai.CameraBoardSocket.CAM_A)
        self.camRgb.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
        
        self.monoLeft.setResolution(dai.MonoCameraProperties.SensorResolution.THE_400_P)
        self.monoLeft.setCamera("left")
        self.monoRight.setResolution(dai.MonoCameraProperties.SensorResolution.THE_400_P)
        self.monoRight.setCamera("right")
       
        self.depthEnc.setDefaultProfilePreset(25, dai.VideoEncoderProperties.Profile.H264_MAIN)
        self.depthEnc.setKeyframeFrequency(30)  # Insert a keyframe every 30 frames

        self.colorEnc.setDefaultProfilePreset(25, dai.VideoEncoderProperties.Profile.H264_MAIN)
        self.colorEnc.setKeyframeFrequency(30)  # Insert a keyframe every 30 frames
        self.camRgb.setFps(30)

        # print camera resolution
        # print(self.camRgb.getResolutionSize())

        # Create a node that will produce the depth map (using disparity output as it's easier to visualize depth this way)
        self.depth.setDefaultProfilePreset(dai.node.StereoDepth.PresetMode.HIGH_ACCURACY)
        # Options: MEDIAN_OFF, KERNEL_3x3, KERNEL_5x5, KERNEL_7x7 (default)
        self.depth.initialConfig.setMedianFilter(dai.MedianFilter.KERNEL_7x7)
        self.depth.setLeftRightCheck(lr_check)
        self.depth.setExtendedDisparity(extended_disparity)
        self.depth.setSubpixel(subpixel)

        config = self.depth.initialConfig.get()
        config.postProcessing.speckleFilter.enable = True
        config.postProcessing.speckleFilter.speckleRange = 50
        config.postProcessing.temporalFilter.enable = True
        config.postProcessing.spatialFilter.enable = True
        config.postProcessing.spatialFilter.holeFillingRadius = 2
        config.postProcessing.spatialFilter.numIterations = 1
        config.postProcessing.thresholdFilter.minRange = 400
        config.postProcessing.thresholdFilter.maxRange = 15000
        config.postProcessing.decimationFilter.decimationFactor = 1

        self.depth.initialConfig.set(config)
        self.depth.initialConfig.setConfidenceThreshold(80)

        # Linking
        self.monoLeft.out.link(self.depth.left)
        self.monoRight.out.link(self.depth.right)

        self.depth.disparity.link(self.depthEnc.input)
        self.depthEnc.bitstream.link(self.depthOut.input)

        self.camRgb.video.link(self.colorEnc.input)
        self.colorEnc.bitstream.link(self.colorOut.input)

        self.device = dai.Device(self.pipeline)

        # calib_data = self.device.readCalibration()

        # Get the intrinsics for the left mono camera
        # intrinsics = calib_data.getCameraIntrinsics(dai.CameraBoardSocket.CAM_B, self.monoLeft.getResolutionWidth(), self.monoLeft.getResolutionHeight())
        
        # The intrinsics matrix is in the format:
        # [fx  0 cx]
        # [ 0 fy cy]
        # [ 0  0  1]
        # fx and fy are the focal lengths in pixels
        # focal_length_x = intrinsics[0][0]
        # focal_length_y = intrinsics[1][1]

        # print(f"Focal Length X: {focal_length_x}")
        # print(f"Focal Length Y: {focal_length_y}")

        self.depth_q = self.device.getOutputQueue(name="depth", maxSize=1, blocking=True)
        self.color_q = self.device.getOutputQueue(name="color", maxSize=1, blocking=True)

        # spin off a thread to create depth frame
        threading.Thread(target=self.create_depth_frame, daemon=True).start()

        self.pts = 0

    def create_depth_frame(self):
        global depth_packet
        pts = 0
        while not self.done:
            if self.depth_q.has():
                packet = Packet(self.depth_q.get().getData())
                packet.pts = pts
                packet.time_base = Fraction(1, 30) 
                pts += 1
                depth_packet = packet
            else:
                time.sleep(0.01)


    def __del__(self):
        self.done = True
        self.device.close()

    async def recv(self):
        packet = None
        # Emptying queue
        while packet is None:
            if self.color_q.has():        
                packet = Packet(self.color_q.get().getData())
                packet.pts = self.pts
                packet.time_base = Fraction(1, 30) 
                self.pts += 1
            else:
                await asyncio.sleep(0.01)
        
        return packet

class DepthVideoTransformTrack(MediaStreamTrack):
    """
    A video stream track that transforms frames from an another track.
    """

    kind = "video"

    def __init__(self):
        super().__init__()  # don't forget this!

    async def recv(self):
        global depth_packet
        while depth_packet is None:
            await asyncio.sleep(0.01)

        temp = depth_packet
        depth_packet = None
        return temp

async def send_data(channel):
    pass
    # tracking_reader = SDataLib.SDataPositionSystem(MAP_NAME_TRACKING, False)
    # imu_reader = SDataLib.SDataIMU(MAP_NAME_IMU, False)
    
    # tracking_state  = SDataLib.positionSystem_t()
    # imu_state  = SDataLib.imuData_t()

    # drive_reader = SDataLib.SDataDriveSystemState(MAP_NAME_DRIVE_SYSTEM, False)
    # drive_state  = SDataLib.driveSystemState_t()

    # camera_reader = SDataLib.SDataCameraFeed(MAP_NAME_CAMERA, False)
    # camera_frame = SDataLib.camera_feed_t()

    # data_json = {
    #     "image": {
    #         "width": 640,
    #         "height": 480,
    #         "data": {}
    #     },
    #     "drive_system": [
    #         {
    #             "velocity": 0,
    #             "position": 0,
    #             "vBusVoltage": 0,
    #             "state": 0,
    #             "error": False
    #         },
    #         {
    #             "velocity": 0,
    #             "position": 0,
    #             "vBusVoltage": 0,
    #             "state": 0,
    #             "error": False
    #         },
    #         {
    #             "velocity": 0,
    #             "position": 0,
    #             "vBusVoltage": 0,
    #             "state": 0,
    #             "error": False
    #         },
    #         {
    #             "velocity": 0,
    #             "position": 0,
    #             "vBusVoltage": 0,
    #             "state": 0,
    #             "error": False
    #         }
    #     ],
    #     "position": {
    #         "slam": {
    #             "position" : {
    #                 "x": 0,
    #                 "y": 0,
    #                 "z": 0
    #             },
    #             "orientation": {
    #                 "yaw": 0,
    #                 "pitch": 0,
    #                 "roll": 0
    #             },
    #             "position_status": 2,
    #             "timestamp": 0
    #         },
    #         "position_status": 2
    #     }
    # }

    # counter = 0
    # timeinterval = 10

    # while True:
    #     counter += 1
    #     await asyncio.sleep(.1)
    #     if counter % timeinterval == 0:
    #         # get less frequent data
    #         if not drive_reader.getData(drive_state):
    #             print("Failed to get drive data")
    #             continue

    #         for i in range(4):
    #             axis = drive_state.getAxis(i)
    #             data_json["drive_system"][i]["velocity"] = axis.velocity
    #             data_json["drive_system"][i]["position"] = axis.position
    #             data_json["drive_system"][i]["vBusVoltage"] = axis.vBusVoltage
    #             data_json["drive_system"][i]["state"] = axis.state
    #             data_json["drive_system"][i]["error"] = axis.error

          
    #     if not tracking_reader.getData(tracking_state):
    #         print("Failed to get imu data")
    #         continue

    #     if not imu_reader.getData(imu_state):
    #         print("Failed to get imu data")
    #         continue

    #     data_json["position"]["slam"]["position"]["x"] = tracking_state.position.x
    #     data_json["position"]["slam"]["position"]["y"] = tracking_state.position.y
    #     data_json["position"]["slam"]["position"]["z"] = tracking_state.position.z
    #     data_json["position"]["slam"]["position_status"] = tracking_state.status
    #     data_json["position"]["slam"]["timestamp"] = tracking_state.timestamp
    #     data_json["position"]["slam"]["orientation"]["yaw"] = imu_state.angles.yaw
    #     data_json["position"]["slam"]["orientation"]["pitch"] = imu_state.angles.pitch
    #     data_json["position"]["slam"]["orientation"]["roll"] = imu_state.angles.roll
        
    #     channel.send(json.dumps(data_json))

async def index(request):
    content = open(os.path.join(ROOT, "index.html"), "r").read()
    return web.Response(content_type="text/html", text=content)

async def javascript(request):
    content = open(os.path.join(ROOT, "client.js"), "r").read()
    return web.Response(content_type="application/javascript", text=content)

async def path_html(request):
    content = open(os.path.join(ROOT, "templates/path.html"), "r").read()
    return web.Response(content_type="text/html", text=content)

async def offer(request):
    print("got offer")
    params = await request.json()
    offer = RTCSessionDescription(sdp=params["sdp"], type=params["type"])

    pc = RTCPeerConnection()
    pc_id = "PeerConnection(%s)" % uuid.uuid4()
    pcs.add(pc)

    # add video track
    color_video = ColorVideoTransformTrack()
    depth_video = DepthVideoTransformTrack()    

    color_track = pc.addTrack(color_video)
    depth_track = pc.addTrack(depth_video)
    
    force_codec(pc, color_track, "video/H264")    
    force_codec(pc, depth_track, "video/H264")    

    # add data track
    channel = pc.createDataChannel("status_feed")
    print(channel, "-", "created by local party")

    @channel.on("open")
    async def on_open():
        print("Async Data channel is open")
        await send_data(channel)

    @channel.on("message")
    async def on_message(message):
        # if message is ping, respond with pong
        print("async message: ", message)   

        if isinstance(message, str) and message.startswith("ping"):
            channel.send("pong" + message[4:])
        elif isinstance(message, str) and message.startswith("JSON"):
            json_data = json.loads(message[4:])
            if json_data.get("type") == "camera":
                # call setCrop in video track
                pass
                # await robot_command.setJoystick(json_data)



    def log_info(msg, *args):
        print(pc_id + " " + msg, *args)

    log_info("Created for %s", request.remote)

    @pc.on("datachannel")
    def on_datachannel(channel):

        @channel.on("open")
        def on_open():
            log_info("poop Data channel is open")

    @pc.on("connectionstatechange")
    async def on_connectionstatechange():
        log_info("Connection state is %s", pc.connectionState)
        if pc.connectionState == "failed":
            await pc.close()
            pcs.discard(pc)
       
    @pc.on("track")
    def on_track(track):
        print("Track received: ", track.kind)

        if track.kind == "data":
            asyncio.ensure_future(send_data(channel))


        @track.on("ended")
        async def on_ended():
            log_info("Track %s ended", track.kind)

    # handle offer
    await pc.setRemoteDescription(offer)

    # send answer
    answer = await pc.createAnswer()
    await pc.setLocalDescription(answer)

    return web.Response(
        content_type="application/json",
        text=json.dumps(
            {"sdp": pc.localDescription.sdp, "type": pc.localDescription.type}
        ),
    )

async def getSpatialMap(request):
    print("getSpatialMap")
    # get list of meshes from map folder
    meshes = []
    for file in os.listdir(MAP_PATH):
        if file.endswith(".obj"):
            meshes.append(file)
    

    # Incrament the map request counter
    # map_request_counter += 1
    return web.Response(
        content_type="application/json",
        text=json.dumps(
            {"map": meshes}
        ),
    )

async def on_shutdown(app):
    # close peer connections
    coros = [pc.close() for pc in pcs]
    await asyncio.gather(*coros)
    pcs.clear()


if __name__ == "__main__":

    app = web.Application()
    app.on_shutdown.append(on_shutdown)
    app.router.add_get("/path.html", path_html)
    app.router.add_get("/client.js", javascript)
    app.router.add_get("/request_spatial_map", getSpatialMap)
   
    app.add_routes([web.get('/', index),
                web.static('/static', './static')])

    app.router.add_post("/offer", offer)
    web.run_app(
        app, access_log=None, host="0.0.0.0", port=8080, ssl_context=None
    )
    