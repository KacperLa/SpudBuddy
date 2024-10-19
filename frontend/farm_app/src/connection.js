import React, { useState, useEffect } from 'react';
import { faSignal, faFile, faLeaf, faDroplet, faUpRightAndDownLeftFromCenter } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import Button from 'react-bootstrap/Button';

import Modal from 'react-bootstrap/Modal';

import Col from 'react-bootstrap/esm/Col';
import Row from 'react-bootstrap/Row';

// import style
import './connection.css';

function FileDownloadManager(props) {
    const [downloadStatus, setDownloadStatus] = useState(null);
    const [buffer_data, setBufferData] = useState(new Uint8Array(0));
    const [file_type, setFileType] = useState(0);
    const [buffer_checksum, setBufferChecksum] = useState(0);
    const [message_counter, setMessageCounter] = useState(0);
    const [message_length, setMessageLength] = useState(0);


    useEffect(() => {
        console.log("Packet received:", props.packet);
        if (props.packet != null) { 
            processPacket(props.packet);
        }
    } , [props.packet]);

    const resetBuffer = () => {
        setBufferData(new Uint8Array(0));
        setFileType(0);
        setBufferChecksum(0);
        setMessageCounter(0);
        setMessageLength(0);
    }

    const calculatePacketChecksum = (payload) => {
        let checksum = 0;
        // python equivalent: checksum = sum(packet) % 256
        // sum(data) % 256
        for (let i = 0; i < payload.length; i++) {
            // interpete each byte as unsigned 8 bit integer
            checksum += payload[i];
        }
        checksum = checksum % 256;
        return checksum;
    }

    const processDataAsJson = () => {
        const data = new TextDecoder().decode(buffer_data);
        console.log("Received data:", data);
        try {   
            props.setFarmData(JSON.parse(data));
        }
        catch (error) {
            console.log("Error parsing JSON data");
            resetBuffer();
        }
    }

    const processDataAsCsv = () => {
        console.log("Received csv data");
        const data = new TextDecoder().decode(buffer_data);
        console.log("Received data:", data);
        // download the csv data
        try {
            // download the csv data
            const blob = new Blob([data], {type: 'text/csv'});
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = 'moisture_data.csv';
            a.click();
        }
        catch (error) {
            console.log("Error parsing CSV data");
            resetBuffer();
        }
    }

    const processHeader = (packet) => {
        // New Header
        // Reset all the variables
        resetBuffer();
        
        // --------------------------------
        // Check header packet checksum
        console.log("Header packet:", packet);

        let message_checksum = packet.getUint8(packet.buffer.byteLength - 1);
        let packet_checksum  = calculatePacketChecksum(new Uint8Array(packet.buffer.slice(0, -1)));
        if (message_checksum !== packet_checksum) {
            console.log("Header checksum error", message_checksum, packet_checksum);
            resetBuffer();
            return;
        }

        // --------------------------------
        // Parse the header
        setFileType(packet.getUint8(1));
        setMessageLength(packet.getUint8(2));
        setBufferChecksum(packet.getUint8(3));

        console.log("File type:",        file_type);
        console.log("Message length:",   message_length);
        console.log("Buffer checksum:",  buffer_checksum);
        console.log("Message checksum:", message_checksum);

        // update modal
        setDownloadStatus("Downloading data 0 of " + message_length);
    };

    const processPayload = (packet) => {
        // Check payload packet checksum
        let message_checksum = packet.getUint8(packet.buffer.byteLength - 1);
        let packet_checksum  = calculatePacketChecksum(new Uint8Array(packet.buffer.slice(0, -1)));
        if (message_checksum !== packet_checksum) {
            console.log("Payload checksum error");
            resetBuffer();
            return;
        }

        // Check packet index
        let packet_index = packet.getUint8(1);
        console.log("Current message: ", packet_index, " of ", message_length);
        if (packet_index !== message_counter) {
            console.log("Packet index error");
            resetBuffer();
            return;
        }

        // If all goes well then append the payload to the buffer
        console.log("Message: ", packet_index, " of ", message_length);
        let payload = new Uint8Array(packet.buffer.slice(2, -1));  
        setBufferData(new Uint8Array([...buffer_data, ...payload]));
        
        // Increment the message counter
        setMessageCounter(message_counter + 1);

        // update modal
        setDownloadStatus("Downloading data " + message_counter + " of " + message_length);
    };

    const processFooter = (packet) => {
        // Check footer packet checksum
        setDownloadStatus("Downloading complete.");


        switch (file_type) {
            case 1: // Json data
                processDataAsJson();
                break;
            case 2: // CSV data
                processDataAsCsv();
                break;
            default:
                console.log("Unknown file type");
        }

        setDownloadStatus(null);
        // reset the buffer
        resetBuffer();
    };

    const processPacket = (packet) => {
        // Check the first byte to see if what type of data is being sent
        switch (packet.getUint8(0)) {
            case 1:
                // Header data
                processHeader(packet);
                break;
            case 2:
                // Payload data
                processPayload(packet);
                break;
            case 3:
                // Footer
                processFooter(packet);
                break;
            default:
                console.log("Unknown packet type");
                resetBuffer();
        } 
    };

    return (
        <div
                className="modal show"
                style={{ 
                        display: downloadStatus != null ? 'block' : 'none',
                        position: 'initial'
                    }}
            >
                <Modal.Dialog>
                    <Modal.Header closeButton>
                    <Modal.Title>File Transfer Manager</Modal.Title>
                    </Modal.Header>

                    <Modal.Body>
                    <p>{ downloadStatus }</p>
                    </Modal.Body>

                    <Modal.Footer>
                    </Modal.Footer>
                </Modal.Dialog>
            </div>
    );
}

function ConnectivityComponent(props) {
    const [server, setServer] = useState(null);
    const [packet, setPacket] = useState(null);

    const [heartBeat, setHeartBeat] = useState(null);    

    // Setup watchdog timer to check if the server is still connected
    // useEffect(() => {
    //     const interval = setInterval(() => {
    //         if (server) {
    //             if (heartBeat == null) {
    //                 alert("Connected to robot lost");
    //                 setServer(null);
    //             } else {
    //                 setHeartBeat(null);
    //             }
    //         }
    //         else {
    //             console.log("Server not connected");
    //         }
    //     }, 10000);

    //     // Cleanup interval on component unmount
    //     return () => clearInterval(interval);
    // }, [server]);
    
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

    // When prop.desriedPos changes, send the new desired position to the robot
    useEffect(() => {
        if (props.robotCmd != null)
        {
            sendRobotCmd(props.robotCmd);
        }
    }, [props.robotCmd]);

    useEffect(() => {
        if (props.datatoSend != null)
        {
            requestData(3, props.datatoSend);
        }
    }, [props.datatoSend]);

    function requestData(value, data=null) {
        console.log("Requesting data", value);

        if (server) {
            const fetchDeviceInfoService = async () => {
                try {
                    const deviceInfoService = await server.getPrimaryService(0x181A); // Device Information
                    console.log(deviceInfoService);

                    const characteristic = await deviceInfoService.getCharacteristic('dc5d258b-ae55-48d3-8911-7c733b658cfd');
                    if (data != null) {
                        console.log("Sending data request");
                        console.log(data);
                        await characteristic.writeValue(data);
                    }
                    else
                    {
                        await characteristic.writeValue(new Uint8Array([value]));
                    }
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
                                
                deviceInfoService.getCharacteristic('16cbec17-9876-490c-bc71-85f24643a7d9').then(characteristic => {
                    characteristic.startNotifications().catch(error => {
                        console.error('Error starting notifications:', error);
                    });
                    characteristic.addEventListener('characteristicvaluechanged', (event) => {
                        // retrive data as a chunk of 20 utf-8 bytes
                        console.log("Chunk event:", event.target.value);
                        setPacket(event.target.value);
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
                        setHeartBeat(true);
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
            <FileDownloadManager packet={packet} setFarmData={props.setFarmData}/>
            
            <Button
                size="lg"
                onClick={createConnection}
                variant={(server != null) ? "success" : "outline-light"}
                style={{
                    margin: '0px 5px',
                }}
            >
                <div
                    style={{
                        color: 'white',
                    }}
                >
                    <Row>
                        <Col xs={9} className="button-text">
                            Connect Robot
                        </Col>
                        <Col xs={3} className="button-icon">
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
            <Button
                size="lg"
                onClick={() => sendRobotCmd([6, 0, 0, 0, 0])}
                variant="outline-light"
                style={{
                    margin: '0px 5px',
                }}
            >
                <div
                    style={{
                        color: 'white',
                    }}
                >
                    <FontAwesomeIcon icon={faUpRightAndDownLeftFromCenter}/>
                </div>
            </Button>
        </div>
    );
}

export default ConnectivityComponent ;
