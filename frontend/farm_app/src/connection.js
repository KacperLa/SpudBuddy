import React, { useState, useEffect, useRef } from 'react';
import { faSignal } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import Button from 'react-bootstrap/Button';

import Col from 'react-bootstrap/esm/Col';
import Row from 'react-bootstrap/Row';

function ConnectivityComponent(props) {
    const [server, setServer] = useState(null);
    
    const [pingInterval, setPingInterval] = useState(null);    

    // when prop.desriedPos changes, send the new desired position to the robot
    useEffect(() => {
        if (server) {
            const fetchDeviceInfoService = async () => {
                try {
                    const deviceInfoService = await server.getPrimaryService(0x181A); // Device Information
                    console.log(deviceInfoService);

                    const characteristic = await deviceInfoService.getCharacteristic('5bfd1e3d-e9e6-4272-b3fe-0be36b98fb9c');
                    await characteristic.writeValue(new Uint16Array(props.desiredPos));
                    console.log(new Uint16Array(props.desiredPos));
                    console.log("Sent desired position to robot:", props.desiredPos);
                } catch (error) {
                    console.error('Error:', error);
                }
            };

            fetchDeviceInfoService();
        } else {
            console.log("Server not connected");
        }
    }, [props.desiredPos, server]);

    const createConnection = async () => {
        // Check if the Bluetooth API is available
        if (!navigator.bluetooth) {
            alert('Web Bluetooth API is not available.\nPlease make sure the Web Bluetooth flag is enabled in chrome://flags.');
            return;
        }

        try {
            const device = await navigator.bluetooth.requestDevice({
                filters: [
                    {
                        namePrefix: "FarmBot",
                    },
                ],
                optionalServices: [0x181A],
            });
            // Proceed with connecting to the device and using it
            
            if (!device) {
                return;
            }

            const newServer = await device.gatt?.connect();

            // print all characteristics
            const services = await newServer.getPrimaryServices();
            services.forEach(async service => {
                const characteristics = await service.getCharacteristics();
                console.log('Service: ' + service.uuid);
                characteristics.forEach(characteristic => {
                    console.log('Characteristic: ' + characteristic.uuid);
                });
            });
            
            try {
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
                    });
                }).catch(error => {
                    console.error('Error accessing characteristic:', error);
                });

            } catch (error) {
                console.error('Error accessing services:', error);
            }
        
            setServer(newServer);
            
        } catch (error) {
            console.error('Bluetooth requestDevice error:', error);
        }
    };

    return (
        <div>        
            <Button size="lg" onClick={createConnection} variant="outline-light" style={{width:'100%'}}>
                <div
                    style={{
                        color: 'white',
                    }}
                >
                    <Row>
                        <Col xs={9}>
                            Connect Robot
                        </Col>
                        <Col xs={3} >
                                <FontAwesomeIcon icon={faSignal}/>
                        </Col>
                    </Row>
                </div>
            </Button>
        </div>
    );
}

export default ConnectivityComponent ;
