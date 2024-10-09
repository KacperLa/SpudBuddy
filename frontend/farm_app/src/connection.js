import React, { useState, useEffect } from 'react';
import { faSignal, faFile, faLeaf, faDroplet } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import Button from 'react-bootstrap/Button';

import Col from 'react-bootstrap/esm/Col';
import Row from 'react-bootstrap/Row';

function ConnectivityComponent(props) {
    const [server, setServer] = useState(null);
    
    // const [pingInterval, setPingInterval] = useState(null);    

    function sendRobotCmd(robotCmd) {
        if (server) {
            const fetchDeviceInfoService = async () => {
                try {
                    const deviceInfoService = await server.getPrimaryService(0x181A); // Device Information
                    console.log(deviceInfoService);

                    const characteristic = await deviceInfoService.getCharacteristic('5bfd1e3d-e9e6-4272-b3fe-0be36b98fb9c');
                    await characteristic.writeValue(new Uint16Array(robotCmd));
                    console.log(new Uint16Array(robotCmd));
                    console.log("Sent desired position to robot:", robotCmd);
                } catch (error) {
                    console.error('Error:', error);
                }
            };

            fetchDeviceInfoService();
        } else {
            console.log("Server not connected");
        }
    };

    // when prop.desriedPos changes, send the new desired position to the robot
    useEffect(() => {
        sendRobotCmd(props.robotCmd);
    }, [props.robotCmd]);

    function requestData(value) {
        console.log("Requesting data", value);

        if (server) {
            const fetchDeviceInfoService = async () => {
                try {
                    const deviceInfoService = await server.getPrimaryService(0x181A); // Device Information
                    console.log(deviceInfoService);

                    const characteristic = await deviceInfoService.getCharacteristic('dc5d258b-ae55-48d3-8911-7c733b658cfd');
                    await characteristic.writeValue(new Uint16Array([value]));
                    console.log("Sent File request");
                } catch (error) {
                    console.error('Error:', error);
                }
            };

            fetchDeviceInfoService();
        } else {
            console.log("Server not connected");
        }
    };

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
            // const services = await newServer.getPrimaryServices();
            // services.forEach(async service => {
            //     const characteristics = await service.getCharacteristics();
            //     console.log('Service: ' + service.uuid);
            //     characteristics.forEach(characteristic => {
            //         console.log('Characteristic: ' + characteristic.uuid);
            //     });
            // });
            
            try {
                const deviceInfoService = await newServer.getPrimaryService(0x181A); // Device Information
                // Empty utf-8 buffer
                let buffer_data = new Uint8Array(0);
                let file_type = 0;
                let buffer_checksum = 0;
                let message_counter = 0;
                let message_length = 0;

                deviceInfoService.getCharacteristic('16cbec17-9876-490c-bc71-85f24643a7d9').then(characteristic => {
                    characteristic.startNotifications().catch(error => {
                        console.error('Error starting notifications:', error);
                    });
                    characteristic.addEventListener('characteristicvaluechanged', (event) => {
                        // retrive data as a chunk of 20 utf-8 bytes
                        console.log("Chunk event:", event.target.value);

                        // Check the first byte to see if what type of data is being sent
                        if (event.target.value.getUint8(0) === 1) {
                            // Header
                            console.log("Header received");
                            file_type = event.target.value.getUint8(1);
                            message_length = event.target.value.getUint8(2);
                            buffer_checksum = event.target.value.getUint8(3);
                            let message_checksum =  event.target.value.getUint8(4);;
                            message_counter = 0;
                            console.log("File type:", file_type);
                            console.log("Message length:", message_length);
                            console.log("Buffer checksum:", buffer_checksum);
                            console.log("Message checksum:", message_checksum);
                        }

                        if (event.target.value.getUint8(0) === 2) {
                            // Payload data
                            console.log("Payload received");
                            message_counter += 1;
                            let packet_index = event.target.value.getUint8(1);
                            let payload_checksum = event.target.value.getUint8(2);

                            console.log("Message: ", packet_index, " of ", message_length);
                            let payload = new Uint8Array(event.target.value.buffer.slice(3));  
                            buffer_data = new Uint8Array([...buffer_data, ...payload]);
                        }

                        if (event.target.value.getUint8(0) === 3) {
                            // Footer
                            console.log("Footer received");
                    
                            if (file_type === 1) {
                                // Json data
                                const data = new TextDecoder().decode(buffer_data);
                                console.log("Received data:", data);
                                props.setFarmData(JSON.parse(data));
                            } else if (file_type === 2) {
                                // CSV data
                                console.log("Received csv data");
                                const data = new TextDecoder().decode(buffer_data);
                                console.log("Received data:", data);
                                // download the csv data
                                const blob = new Blob([data], {type: 'text/csv'});
                                const url = URL.createObjectURL(blob);
                                const a = document.createElement('a');
                                a.href = url;
                                a.download = 'moisture_data.csv';
                                a.click();
                            }

                          
                           // reset the buffer
                           buffer_data = new Uint8Array(0);
                        }             
                    });
                }).catch(error => {
                    console.error('Error accessing characteristic:', error);
                });

                deviceInfoService.getCharacteristic('35f24b15-aa74-4cfb-a66a-a3252d67c264').then(characteristic => {
                    characteristic.startNotifications().catch(error => {
                        console.error('Error starting notifications:', error);
                    });
                    characteristic.addEventListener('characteristicvaluechanged', (event) => {
                        props.setRobotPos([event.target.value.getUint16(2), event.target.value.getUint16(4), event.target.value.getUint16(0)]);
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
            <Button
                size="lg"
                onClick={createConnection}
                variant="outline-light"
                style={{
                    width:'50%',
                    margin: '0px 5px',
                }}
            >
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
            <Button
                size="lg"
                onClick={() => requestData(0)}
                variant="outline-light"
                style={{
                    width:'10%',
                    margin: '0px 5px',
                }}
            >
                <div
                    style={{
                        color: 'white',
                    }}
                >
                    <FontAwesomeIcon icon={faFile}/>
                </div>
            </Button>
            <Button
                size="lg"
                onClick={() => requestData(1)}
                variant="outline-light"
                style={{
                    width:'10%',
                    margin: '0px 5px',
                }}
            >
                <div
                    style={{
                        color: 'white',
                    }}
                >
                    <FontAwesomeIcon icon={faLeaf}/>
                </div>
            </Button>
            <Button
                size="lg"
                onClick={() => requestData(2)}
                variant="outline-light"
                style={{
                    width:'10%',
                    margin: '0px 5px',
                }}
            >
                <div
                    style={{
                        color: 'white',
                    }}
                >
                    <FontAwesomeIcon icon={faDroplet}/>
                </div>
            </Button>
        </div>
    );
}

export default ConnectivityComponent ;
