import React, { useState, useEffect } from 'react';
import { faSignal, faFile, faLeaf, faFlag, faDroplet, faWater, faUpRightAndDownLeftFromCenter } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import Button from 'react-bootstrap/Button';

import Modal from 'react-bootstrap/Modal';

import Col from 'react-bootstrap/esm/Col';
import Row from 'react-bootstrap/Row';

// import style
import './connection.css';
import { Alert } from 'bootstrap';

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

    const processDataAsCsv = (file_name) => {
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
            a.download = (file_name+".csv");
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
                let file_name = new TextDecoder().decode(new Uint8Array(packet.buffer.slice(1, packet.buffer.byteLength)));
                processDataAsCsv(file_name);
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
                        position: 'fixed',
                        zIndex: 10000,
                       
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

function getTimeStamp() {
    // Get time as an array in [HH,MM,SS] format in 24 hour time
    let time = new Date().toLocaleTimeString().split(/:| /);
    if (time[3] === "PM" && time[0] !== "12") {
        time[0] = Number(time[0]) + 12;
    }
    // Get the current date
    let date = new Date().toLocaleDateString().split('/');
    let day = new Date().getDay();
    console.log("Date: ", date);
    console.log("Time: ", time);
    console.log("weekday: ", new Date().getDay());
    // sec min hour month day year
    time = new Uint8Array([99, Number(time[2]), Number(time[1]), Number(time[0]), day, Number(date[0]), Number(date[1]), Number(date[2])-2000]);
    console.log("Time Stamp: ", time);
    return time;
}

function ConnectivityComponent(props) {
    const [ble_device, setBleDevice] = useState(null);
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

    const sendRobotCmdToClient = async (robotCmd) => {
        try {
            server.getPrimaryService(0x181A).then(service => {
                service.getCharacteristic('5bfd1e3d-e9e6-4272-b3fe-0be36b98fb9c').then(characteristic => {
                    characteristic.writeValue(new Uint16Array(robotCmd));
                    console.log("Sent desired position to robot:", robotCmd);
                }).catch(error => {
                    console.error('Error accessing characteristic:', error);
                });
            }).catch(error => {
                console.error('Error accessing services:', error);
            });
        } catch (error) {
            console.error('Error:', error);
        }
    };

    function sendRobotCmd(robotCmd) {
        if (server) {
            if (!server.connected) {
                console.log("Server not connected");
                return;
            }
            sendRobotCmdToClient(robotCmd);
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

    const fetchDeviceInfoService = async () => {
        try {
            const newServer = await ble_device.gatt.connect();
            setServer(newServer);
        } catch (error) {
            console.error('Bluetooth requestDevice error:', error);
        }
    };

    useEffect(() => {
        if (ble_device) {
            // Check if the device is connected
            if (server) {
                return;
            }
            console.log("Server not connected attempt to reconnect");
            fetchDeviceInfoService();
        } else {
            console.log("Device not set");
        }
    }, [ble_device]);

    useEffect(() => {
        if (server) {
            if (server.connected) {
                console.log("Server connected");
                // print all characteristics
                server.getPrimaryServices().then((services) => {
                    services.forEach(async service => {
                        const characteristics = await service.getCharacteristics();
                        console.log('Service: ' + service.uuid);
                        characteristics.forEach(characteristic => {
                            console.log('Characteristic: ' + characteristic.uuid);
                        });
                    });

                    server.getPrimaryService(0x181A).then(service => {
                        service.getCharacteristic('35f24b15-aa74-4cfb-a66a-a3252d67c264').then(characteristic => {
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
                    }).catch(error => {
                        console.error('Error accessing services:', error);
                    });

                    server.getPrimaryService(0x181A).then(service => {
                        service.getCharacteristic('16cbec17-9876-490c-bc71-85f24643a7d9').then(characteristic => {
                            characteristic.startNotifications().catch(error => {
                                console.error('Error starting notifications:', error);
                            });
                            characteristic.addEventListener('characteristicvaluechanged', (event) => {
                                // retrive data as a chunk of 20 utf-8 bytes
                                console.log("Chunk event:", event.target.value);
                                setPacket(event.target.value);
                            });
                            requestData(3, getTimeStamp());
                            requestData(0);
                        }).catch(error => {
                            console.error('Error accessing characteristic:', error);
                        });
                    }).catch(error => {
                        console.error('Error accessing services:', error);
                    });


                }).catch(error => {
                    console.log("Server disconnected, attempting to reconnect...");
                    ble_device.gatt.connect().then((newServer) => {
                      setServer(newServer);
                    }).catch(error => {
                      console.error('Error reconnecting to server:', error);
                    });
                });
            }
        }
    }, [server]);

    function requestData(value, data=null) {
        console.log("Requesting data", value);

        if (ble_device) {
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
            
            console.log("Device:", device);
            setBleDevice(device);
            
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
                <div className="button-content">
                    <FontAwesomeIcon icon={faSignal} />
                    <span className="button-text">Connect Robot</span>
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
                <div className="button-content">
                    <FontAwesomeIcon className="button-icon" icon={faFile} />
                    <span className="button-label">Reload data from robot</span>
                </div>
            </Button>
            <Button
                size="lg"
                onClick={() => requestData(5)}
                variant="outline-light"
                style={{
                    margin: '0px 5px',
                }}
            >
                <div className="button-content">
                    <FontAwesomeIcon className="button-icon" icon={faWater} />
                    <span className="button-label">Download moisture data</span>
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
                <div className="button-content">
                    <FontAwesomeIcon className="button-icon" icon={faFlag} />
                    <span className="button-label">Download mission data</span>
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
                <div className="button-content">
                    <FontAwesomeIcon className="button-icon" icon={faDroplet} />
                    <span className="button-label">Download watering data</span>
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
                <div className="button-content">
                    <FontAwesomeIcon className="button-icon" icon={faUpRightAndDownLeftFromCenter} />
                    <span className="button-label">Calibrate gantry size</span>
                </div>
            </Button>
        </div>
    );
}

export default ConnectivityComponent ;
