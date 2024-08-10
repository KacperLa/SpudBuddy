import React, {useState, useEffect, useRef} from 'react';
import ReactDOM from 'react-dom/client';
import { createRoot } from 'react-dom/client'
import { Canvas, useThree  } from '@react-three/fiber'
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls";
import reportWebVitals from './reportWebVitals';
import Button from 'react-bootstrap/Button';
import Dropdown from 'react-bootstrap/Dropdown';
import ButtonGroup from 'react-bootstrap/ButtonGroup';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import { faLocationCrosshairs } from '@fortawesome/free-solid-svg-icons'
import { faCarBattery } from '@fortawesome/free-solid-svg-icons'
import { faSignal } from '@fortawesome/free-solid-svg-icons'
import { faPowerOff } from '@fortawesome/free-solid-svg-icons'
import { faEye } from '@fortawesome/free-solid-svg-icons'

// import css styles
import './index.css';

// import custom components
import Joy from './joy.js';
import WebRTCComponent from './connection.js';

function App() {
  // define xPos and yPos as a array of two elements
  const [pos, setPos] = useState([0, 0]);
  const [zoom, setZoom] = useState(false);
  const [selectedMode, setSelectedMode] = useState('Mode Selection');
  const videoRef = useRef(null);
  const depthRef = useRef(null);


  function handleMove(event) {
    setPos([event.x, event.y]);
  }
  
  function handleStop(event) {
    setPos([0, 0]);
  }

  const connect = async () => {
    const device = await navigator.bluetooth.requestDevice({
      filters: [
        {
          namePrefix: "FarmBot",
        },
      ],
      // Philips Hue Light Control Service
      optionalServices: [0x181A],
    });
    const server = await device.gatt?.connect();
      
    const service = await server.getPrimaryService(
      0x181A
    );

    // set a call back function to handle the data
    service.getCharacteristic(0x2A6E).then(characteristic => {
      characteristic.startNotifications();
      characteristic.addEventListener('characteristicvaluechanged', (event) => {
        console.log("evnet:", event.target.value.getUint8(0));
      });
    }); 
  };



  return ( 
    <>
        <div className="fixed-top" style={{zIndex: 10000}}>
          <Row style={{padding: '4px'}}>
              <Col sx={12}>
                <Row>
                  <Col>
                    <Row>
                      <Col>
                        <Dropdown>
                          <Dropdown.Toggle size="lg" variant="outline-light" id="dropdown-basic">
                            {selectedMode} 
                          </Dropdown.Toggle>

                          <Dropdown.Menu>
                            <Dropdown.Item onClick={() => setSelectedMode('Keep Location')}>Keep Location</Dropdown.Item>
                            <Dropdown.Item onClick={() => setSelectedMode('Manual Drive')}>Manual Drive</Dropdown.Item>
                            <Dropdown.Item onClick={() => setSelectedMode('Path Follow')}>Path Follow</Dropdown.Item>
                            <Dropdown.Item onClick={() => setSelectedMode('Autonomous Explore')}>Autonomous Explore</Dropdown.Item>
                          </Dropdown.Menu>
                        </Dropdown>
                      </Col>
                      <Col>
                        <ButtonGroup aria-label="Speed Selection">
                          <Button size="lg" variant="outline-light">Slow</Button>
                          <Button variant="success">Normal</Button>
                          <Button size="lg" variant="outline-light">Fast</Button>
                        </ButtonGroup>
                      </Col>
                    </Row>
                                
                  </Col>
                  <Col xs={3}>
                    <Row style={{padding: '0px'}}>
                    <Col style={{padding: '0px 2px'}}>
                        <Button size="lg" onClick={connect} variant="outline-light" style={{width:'100%'}}>
                          <FontAwesomeIcon icon={faEye}/>
                        </Button>
                      </Col>
                      <Col style={{padding: '0px 2px'}}>
                        <Button size="lg" variant="outline-light" style={{width:'100%'}}>
                          <FontAwesomeIcon icon={faLocationCrosshairs}/>
                        </Button>
                      </Col>
                      <Col style={{padding: '0px 2px'}}>
                        <Button size="lg" variant="outline-light" style={{width:'100%'}}>
                          <FontAwesomeIcon icon={faCarBattery}/>
                        </Button>
                      </Col>
                      <Col style={{padding: '0px 2px'}}>
                        <WebRTCComponent joyXY={pos} zoom_level={zoom} depth_ref={depthRef} video_ref={videoRef} />
                      </Col>
                      <Col style={{padding: '0px 2px'}}>
                        <Button size="lg" variant="outline-light" style={{width:'100%'}}>
                          <FontAwesomeIcon icon={faPowerOff}/>
                        </Button>
                      </Col>
                    </Row>
                  </Col>
                  
                </Row>
              </Col>
              <Col xs={2} style={{padding: '0px 15px'}}>
                <Button size="lg" variant="danger" style={{width: '100%'}}>
                  ESTOP
                </Button>
              </Col>
          </Row>
        </div>
        

        <div id="fullscreen-container" style={{color: 'black', background: 'black'}}>
            <Joy handleMove={handleMove} handleStop={handleStop} />
        </div>

    </>
  )
};


createRoot(document.getElementById('root')).render(<App />)
reportWebVitals();
