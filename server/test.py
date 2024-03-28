import argparse
import asyncio
import json
import logging
import os
import ssl
import uuid

from aiohttp import web
from aiortc import MediaStreamTrack, RTCRtpSender, RTCPeerConnection, RTCSessionDescription, VideoStreamTrack, RTCDataChannelParameters
from aiortc.contrib.media import MediaBlackhole, MediaPlayer, MediaRecorder, MediaRelay
import av
from fractions import Fraction
import threading
import copy

import depthai as dai

import numpy as np
import time

ROOT = os.path.dirname(__file__)
MAP_NAME_CAMERA = 'robot_camera'

logger = logging.getLogger("pc")
pcs = set()
relay = MediaRelay()


class VideoCamera(object):
    def __init__(self):
        self.pipeline = dai.Pipeline()

        # Define sources and output
        self.camRgb = self.pipeline.create(dai.node.ColorCamera)
        self.videoEnc = self.pipeline.create(dai.node.VideoEncoder)
        self.xout = self.pipeline.create(dai.node.XLinkOut)

        self.xout.setStreamName('h264')

        # Properties
        self.camRgb.setBoardSocket(dai.CameraBoardSocket.CAM_A)
        self.camRgb.setResolution(dai.ColorCameraProperties.SensorResolution.THE_4_K)
        self.videoEnc.setDefaultProfilePreset(30, dai.VideoEncoderProperties.Profile.H264_MAIN)
        self.videoEnc.setKeyframeFrequency(10)  # Insert a keyframe every 30 frames
        self.videoEnc.setQuality(25)
        self.camRgb.setFps(30)

        # Linking
        self.camRgb.video.link(self.videoEnc.input)
        self.videoEnc.bitstream.link(self.xout.input)

        self.device = dai.Device(self.pipeline)

        self.pts = 0    

    def __del__(self):
        # shutdown the pipeline
        self.device.close()

    async def get_frame(self):
        # Output queue will be used to get the encoded data from the output defined above
        q = self.device.getOutputQueue(name="h264", maxSize=1, blocking=True)

        packet = None
        # Emptying queue
        while packet is None:
            if q.has():        
                packet = av.packet.Packet(q.get().getData())
                packet.pts = self.pts
                packet.time_base = Fraction(1, 30) 
                self.pts += 1
            else:
                await asyncio.sleep(0.01)
        
        return packet

def force_codec(pc, sender, forced_codec):
    kind = forced_codec.split("/")[0]
    codecs = RTCRtpSender.getCapabilities(kind).codecs
    transceiver = next(t for t in pc.getTransceivers() if t.sender == sender)
    transceiver.setCodecPreferences(
        [codec for codec in codecs if codec.mimeType == forced_codec]
    )

class CameraStreamTrack(VideoStreamTrack):
    def __init__(self, camera):
        super().__init__()
        self.camera = camera

    async def recv(self):
        frame = None
        while frame is None:
            frame = await self.camera.get_frame()
        
        return frame

class VideoTransformTrack(MediaStreamTrack):
    """
    A video stream track that transforms frames from an another track.
    """

    kind = "video"

    def __init__(self, track):
        super().__init__()  # don't forget this!
        self.track = track

    async def recv(self):
        frame = await self.track.recv()
        return frame

async def index(request):
    content = open(os.path.join(ROOT, "index.html"), "r").read()
    return web.Response(content_type="text/html", text=content)


async def javascript(request):
    content = open(os.path.join(ROOT, "client.js"), "r").read()
    return web.Response(content_type="application/javascript", text=content)


async def offer(request):
    print("offer")
    params = await request.json()
    offer = RTCSessionDescription(sdp=params["sdp"], type=params["type"])

    pc = RTCPeerConnection()
    pc_id = "PeerConnection(%s)" % uuid.uuid4()
    pcs.add(pc)

    track = pc.addTrack(
                    VideoTransformTrack(CameraStreamTrack(VideoCamera()))
                )
    force_codec(pc, track, "video/H264")    


    def log_info(msg, *args):
        logger.info(pc_id + " " + msg, *args)

    log_info("Created for %s", request.remote)

    @pc.on("datachannel")
    def on_datachannel(channel):
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
        log_info("Track %s received", track.kind)

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
    parser = argparse.ArgumentParser(
        description="WebRTC audio / video / data-channels demo"
    )

    parser.add_argument("--verbose", "-v", action="count")
    args = parser.parse_args()

    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    app = web.Application()
    app.on_shutdown.append(on_shutdown)
    app.router.add_get("/", index)
    app.router.add_get("/client.js", javascript)
    app.router.add_post("/offer", offer)
    web.run_app(
        app, access_log=None, host="0.0.0.0", port=8080, ssl_context=None
    )
    