import asyncio
import json
import os
import uuid

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
MAP_NAME_SETTINGS = '/tmp/robot_settings'
MAP_NAME_IMU  = 'robot_imu'
MAP_NAME_DRIVE_SYSTEM = 'robot_drive_system'
MAP_NAME_TRACKING = 'robot_tracking'
MAP_NAME_CAMERA = 'robot_camera'
MAP_NAME_PC = 'robot_point_cloud'

def force_codec(pc, sender, forced_codec):
    kind = forced_codec.split("/")[0]
    codecs = RTCRtpSender.getCapabilities(kind).codecs
    transceiver = next(t for t in pc.getTransceivers() if t.sender == sender)
    transceiver.setCodecPreferences(
        [codec for codec in codecs if codec.mimeType == forced_codec]
    )

class VideoTransformTrack(MediaStreamTrack):
    """
    A video stream track that transforms frames from an another track.
    """

    kind = "video"

    def __init__(self):
        super().__init__()  # don't forget this!
        self.pipeline = dai.Pipeline()

        # Define sources and output
        self.camRgb = self.pipeline.create(dai.node.ColorCamera)
        self.videoEnc = self.pipeline.create(dai.node.VideoEncoder)
        self.xout = self.pipeline.create(dai.node.XLinkOut)

        self.xout.setStreamName('h264')

        # Properties
        self.camRgb.setBoardSocket(dai.CameraBoardSocket.CAM_A)
        self.camRgb.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
        self.videoEnc.setDefaultProfilePreset(25, dai.VideoEncoderProperties.Profile.H264_MAIN)
        self.videoEnc.setKeyframeFrequency(30)  # Insert a keyframe every 30 frames
        self.camRgb.setFps(30)

        # Linking
        self.camRgb.video.link(self.videoEnc.input)
        self.videoEnc.bitstream.link(self.xout.input)

        self.device = dai.Device(self.pipeline)
        self.q = self.device.getOutputQueue(name="h264", maxSize=1, blocking=True)

        self.pts = 0

    def __del__(self):
        self.device.close()

    async def recv(self):
        packet = None
        # Emptying queue
        while packet is None:
            if self.q.has():        
                packet = Packet(self.q.get().getData())
                packet.pts = self.pts
                packet.time_base = Fraction(1, 30) 
                self.pts += 1
            else:
                await asyncio.sleep(0.01)
        
        return packet

async def send_data(channel):
    tracking_reader = SDataLib.SDataPositionSystem(MAP_NAME_TRACKING, False)
    imu_reader = SDataLib.SDataIMU(MAP_NAME_IMU, False)
    
    tracking_state  = SDataLib.positionSystem_t()
    imu_state  = SDataLib.imuData_t()

    drive_reader = SDataLib.SDataDriveSystemState(MAP_NAME_DRIVE_SYSTEM, False)
    drive_state  = SDataLib.driveSystemState_t()

    data_json = {
        "drive_system": [
            {
                "velocity": 0,
                "position": 0,
                "vBusVoltage": 0,
                "state": 0,
                "error": False
            },
            {
                "velocity": 0,
                "position": 0,
                "vBusVoltage": 0,
                "state": 0,
                "error": False
            },
            {
                "velocity": 0,
                "position": 0,
                "vBusVoltage": 0,
                "state": 0,
                "error": False
            },
            {
                "velocity": 0,
                "position": 0,
                "vBusVoltage": 0,
                "state": 0,
                "error": False
            }
        ],
        "slam": {
            "position" : {
                "x": 0,
                "y": 0,
                "z": 0
            },
            "orientation": {
                "yaw": 0,
                "pitch": 0,
                "roll": 0
            },
            "position_status": 2,
            "timestamp": 0
        },

        "position_status": 1
    }

    counter = 0
    timeinterval = 10

    while True:
        counter += 1
        await asyncio.sleep(.1)
        if counter % timeinterval == 0:
            # get less frequent data
            if not drive_reader.getData(drive_state):
                print("Failed to get drive data")
                continue

            for i in range(4):
                axis = drive_state.getAxis(i)
                data_json["drive_system"][i]["velocity"] = axis.velocity
                data_json["drive_system"][i]["position"] = axis.position
                data_json["drive_system"][i]["vBusVoltage"] = axis.vBusVoltage
                data_json["drive_system"][i]["state"] = axis.state
                data_json["drive_system"][i]["error"] = axis.error

          
        if not tracking_reader.getData(tracking_state):
            print("Failed to get imu data")
            continue

        if not imu_reader.getData(imu_state):
            print("Failed to get imu data")
            continue

        data_json["slam"]["position"]["x"] = tracking_state.position.x
        data_json["slam"]["position"]["y"] = tracking_state.position.y
        data_json["slam"]["position"]["z"] = tracking_state.position.z
        data_json["slam"]["position_status"] = tracking_state.status
        data_json["slam"]["timestamp"] = tracking_state.timestamp
        data_json["slam"]["orientation"]["yaw"] = imu_state.angles.yaw
        data_json["slam"]["orientation"]["pitch"] = imu_state.angles.pitch
        data_json["slam"]["orientation"]["roll"] = imu_state.angles.roll
        
        channel.send(json.dumps(data_json))


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
    print("offer")
    params = await request.json()
    offer = RTCSessionDescription(sdp=params["sdp"], type=params["type"])

    pc = RTCPeerConnection()
    pc_id = "PeerConnection(%s)" % uuid.uuid4()
    pcs.add(pc)

    track = pc.addTrack(
                    VideoTransformTrack()
                )
    
    force_codec(pc, track, "video/H264")    

    # add data track
    channel = pc.createDataChannel("status_feed")
    print(channel, "-", "created by local party")

    @channel.on("open")
    async def on_open():
        print("Data channel is open")
        await send_data(channel)

    def log_info(msg, *args):
        print(pc_id + " " + msg, *args)

    log_info("Created for %s", request.remote)

    @pc.on("datachannel")
    def on_datachannel(channel):

        @channel.on("open")
        def on_open():
            log_info("Data channel is open")

        @channel.on("message")
        def on_message(message):
            if isinstance(message, str) and message.startswith("ping"):
                channel.send("pong" + message[4:])

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
    # app.router.add_get("/joyComponent.mjs", joyComponent)
    # app.router.add_get("/joy.js", joy)
    app.add_routes([web.get('/', index),
                web.static('/static', './static')])

    app.router.add_post("/offer", offer)
    web.run_app(
        app, access_log=None, host="0.0.0.0", port=8082, ssl_context=None
    )
    