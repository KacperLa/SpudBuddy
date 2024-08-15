import React, { useState, useEffect, useRef } from 'react';
import { faSignal } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import Button from 'react-bootstrap/Button';

import OverlayTrigger from 'react-bootstrap/OverlayTrigger';
import Popover from 'react-bootstrap/Popover';
import ButtonGroup from 'react-bootstrap/ButtonGroup';


function ConnectivityComponent(props) {
    const [server, setServer] = useState(null);
    const [dc, setDc] = useState(null);
    const [dcInterval, setDcInterval] = useState(null);
    
    const dataChannelLog = useRef([]);
    const [pingInterval, setPingInterval] = useState(null);    

    const createConnection = async () => {
        const device = await navigator.bluetooth.requestDevice({
            filters: [
                {
                    namePrefix: "FarmBot",
                },
            ],
            optionalServices: [0x181A],
        });
        
        const newServer = await device.gatt?.connect();

        try {
            // const tempService = await newServer.getPrimaryService(0x181A); // Environmental Sensing
            const deviceInfoService = await newServer.getPrimaryService(0x181A); // Device Information

            deviceInfoService.getCharacteristic(0x2A6E).then(characteristic => {
                characteristic.startNotifications().catch(error => {
                    console.error('Error starting notifications:', error);
                });
                characteristic.addEventListener('characteristicvaluechanged', (event) => {
                    console.log("temp event:", event.target.value.getUint16(0));
                });
            }).catch(error => {
                console.error('Error accessing characteristic:', error);
            });

            deviceInfoService.getCharacteristic(0x2A5D).then(characteristic => {
                characteristic.startNotifications().catch(error => {
                    console.error('Error starting notifications:', error);
                });
                characteristic.addEventListener('characteristicvaluechanged', (event) => {
                    props.setRobotPos([event.target.value.getUint16(0), event.target.value.getUint16(2), event.target.value.getUint16(4)]);
                    console.log("pos event:", event.target.value.getUint16(0), event.target.value.getUint16(2), event.target.value.getUint16(4));
                });
            }).catch(error => {
                console.error('Error accessing characteristic:', error);
            });

        } catch (error) {
            console.error('Error accessing services:', error);
            // Handle the error (e.g., retry, notify the user, etc.)
        }
        // set a call back function to handle the data
       
        setServer(newServer);
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
                            </div>
                            <ButtonGroup aria-label="Connection">
                                <Button variant="success" onClick={createConnection}>Connect</Button>
                                {/* <Button variant="danger" onClick={() => stop(pc)}>Disconnect</Button> */}
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

export default ConnectivityComponent ;
