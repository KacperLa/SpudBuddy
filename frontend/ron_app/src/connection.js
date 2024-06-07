import React, { useState, useEffect, useRef } from 'react';
import { faSignal } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import Button from 'react-bootstrap/Button';
import init, { capture_frame_and_create_point_cloud } from 'ron_pc_wasm/ron_wasm';

import OverlayTrigger from 'react-bootstrap/OverlayTrigger';
import Popover from 'react-bootstrap/Popover';
import ButtonGroup from 'react-bootstrap/ButtonGroup';


function WebRTCComponent(props) {
    const [pc, setPc] = useState(null);
    const [dc, setDc] = useState(null);
    const [dcInterval, setDcInterval] = useState(null);
    const [offerSDP, setOfferSDP] = useState('');
    const [answerSDP, setAnswerSDP] = useState('');
    const dataChannelLog = useRef([]);
    const [pingInterval, setPingInterval] = useState(null);
    const [pcInterval, setPcInterval] = useState(null);

    useEffect(
        () => {
            // console.log("component X: ", props.joyXY[0], "Y: ", props.joyXY[1])
            if (dc !== null)
            {
                const joy = {
                    x: props.joyXY[0],
                    y: props.joyXY[1],
                    timestamp: Date.now()
                };
                // console.log(joy);
                dc.send(JSON.stringify(joy));
            }
        },
        [props.joyXY]
    );

    useEffect(
        () => {
            // console.log("component X: ", props.joyXY[0], "Y: ", props.joyXY[1])
            if (dc !== null)
            {
                const zoom_json = {
                    type: "camera",
                    zoom: props.zoom_level,
                    //position: props.joyXY[1],
                    timestamp: Date.now()
                };
                // console.log(joy);
                dc.send("JSON"+JSON.stringify(zoom_json));
            }
        },
        [props.zoom_level]
    );


    useEffect(() => {
        const newPc = createPeerConnection();
        setPc(newPc);

        // Clean up on component unmount
        return () => {
            stop(newPc);
        };
    }, []);




    const width = 640;
    const height = 400;
    const maxPoints = width*height;
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    canvas.width = width;
    canvas.height = height;
    const baseline = 0.750;
    const focalLength = 455.4474182128906;

    const createPeerConnection = () => {
        const config = {
            sdpSemantics: 'unified-plan',
            iceServers: [{ urls: ['stun:stun.l.google.com:19302'] }]
        };
        const newPc = new RTCPeerConnection(config);

        // add transceiver and data channel
        newPc.addTransceiver('video', { direction: 'recvonly' });
        const newDc = newPc.createDataChannel('status_feed');

        newPc.addEventListener('track', (evt) => {
            if (evt.track.kind === 'video') {
                props.video_ref.current.srcObject = evt.streams[0];
            }

            init().then(() => {
                let pcInterval = setInterval(() => {
                    ctx.drawImage(props.video_ref.current, 0, 0, width, height)
                    const imageData = ctx.getImageData(0, 0, width, height);
                    const data = new Uint8Array(imageData.data.buffer);
                    
                    props.updatePointCloud(capture_frame_and_create_point_cloud(width, height, data, focalLength, baseline, maxPoints));
                }, 100);
                setPcInterval(pcInterval);
            });
        });

        newPc.addEventListener('datachannel', (evt) => {
            const newDc = evt.channel;
            setDc(newDc);
            setupDataChannelListeners(newDc);
        });

        return newPc;
    };

    const setupDataChannelListeners = (dataChannel) => {
        let time_start = null;
        const current_stamp = () => {
            if (time_start === null) {
                time_start = new Date().getTime();
                return 0;
            } else {
                return new Date().getTime() - time_start;
            }
        };

        dataChannel.addEventListener('message', (evt) => {
            if (evt.data.substring(0, 4) === 'pong') {
                var elapsed_ms = current_stamp() - parseInt(evt.data.substring(5), 10);
                console.log(' RTT ' + elapsed_ms + ' ms');
                setPingInterval(elapsed_ms);
            } else {
                console.log('Received message:', evt.data);
                const logs = dataChannelLog.current;
                logs.push(evt.data);
                dataChannelLog.current = logs; // Not re-rendering on log update for performance
            }
        });

        dataChannel.addEventListener('close', () => {
            setDcInterval(null);
            setPcInterval(null);
        });

        dataChannel.addEventListener('open', (evt) => {
            console.log('Data channel is open!');
            dataChannel.send('Hello from client!');
            let dcInterval = setInterval(() => {
                var message = 'ping ' + current_stamp();
                dataChannelLog.textContent += '> ' + message + '\n';
                dataChannel.send(message);
            }, 1000);
            setDcInterval(dcInterval);
        });
    };

    const negotiate = async () => {
        try {
            const offer = await pc.createOffer();
            await pc.setLocalDescription(offer);
            setOfferSDP(offer.sdp);
    
            // Use a relative URL, which will be automatically resolved to the full URL by the proxy
            const response = await fetch('/offer', {
                method: 'POST',
                body: JSON.stringify({
                    sdp: offer.sdp,
                    type: offer.type
                }),
                headers: {
                    'Content-Type': 'application/json'
                }
            });
    
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
    
            const answer = await response.json();
            setAnswerSDP(answer.sdp);
            await pc.setRemoteDescription(new RTCSessionDescription(answer));
        } catch (e) {
            console.error('Failed to negotiate:', e);
            alert(`Negotiation failed: ${e.message}`);
        }
    };
    

    const stop = (peerConnection) => {
        if (dc) {
            dc.close();
        }
        peerConnection.getTransceivers().forEach(transceiver => {
            transceiver.stop();
        });
        peerConnection.close();
    };

    return (
        <div>
            {/* <video ref={videoRef} autoPlay style={{ width: '100%' }} /> */}
        
            <>
                <OverlayTrigger
                    trigger="click"
                    key="bottom"
                    placement="bottom"
                    overlay={
                        <Popover id={`popover-positioned-buttom`}>
                        <Popover.Header as="h3">{`Robot Connection`}</Popover.Header>
                        <Popover.Body>
                            <div>
                                <p>Ping: {pingInterval} ms</p>
                                <textarea value={offerSDP} readOnly />
                                <textarea value={answerSDP} readOnly />
                            </div>
                            <ButtonGroup aria-label="Connection">
                                <Button variant="success" onClick={negotiate}>Connect</Button>
                                <Button variant="danger" onClick={() => stop(pc)}>Disconnect</Button>
                            </ButtonGroup>
                            <ul>
                                {dataChannelLog.current.map((log, index) => (
                                    <li key={index}>{log}</li>
                                ))}
                            </ul>

                        </Popover.Body>
                        </Popover>
                        }
                    >
                    <Button size="lg" variant="outline-light" style={{width:'100%'}}>
                        <FontAwesomeIcon icon={faSignal}/>
                    </Button>
                </OverlayTrigger>
            </>
        </div>
    );
}

export default WebRTCComponent;
