import React, { useState, useEffect, useRef } from 'react';

function WebRTCComponent(props) {
    const [pc, setPc] = useState(null);
    const [dc, setDc] = useState(null);
    const [offerSDP, setOfferSDP] = useState('');
    const [answerSDP, setAnswerSDP] = useState('');
    const videoRef = useRef(null);
    const dataChannelLog = useRef([]);

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


    useEffect(() => {
        const newPc = createPeerConnection();
        setPc(newPc);

        // Clean up on component unmount
        return () => {
            stop(newPc);
        };
    }, []);

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
                videoRef.current.srcObject = evt.streams[0];
            }
        });

        newPc.addEventListener('datachannel', (evt) => {
            const newDc = evt.channel;
            setDc(newDc);
            setupDataChannelListeners(newDc);
        });

        return newPc;
    };

    const setupDataChannelListeners = (dataChannel) => {
        dataChannel.addEventListener('message', (evt) => {
            const logs = dataChannelLog.current;
            logs.push(evt.data);
            dataChannelLog.current = logs; // Not re-rendering on log update for performance
        });

        dataChannel.addEventListener('open', (evt) => {
            console.log('Data channel is open!')
            dataChannel.send('Hello from client!');
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
            <video ref={videoRef} autoPlay style={{ width: '100%' }} />
            <div>
                <textarea value={offerSDP} readOnly />
                <textarea value={answerSDP} readOnly />
            </div>
            <button onClick={negotiate}>Negotiate</button>
            <button onClick={() => stop(pc)}>Stop</button>
            <ul>
                {dataChannelLog.current.map((log, index) => (
                    <li key={index}>{log}</li>
                ))}
            </ul>
        </div>
    );
}

export default WebRTCComponent;
