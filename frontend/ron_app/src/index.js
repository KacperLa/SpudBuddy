import React, {useState, useEffect} from 'react';
import ReactDOM from 'react-dom/client';
import { createRoot } from 'react-dom/client'
import { Canvas, useThree  } from '@react-three/fiber'
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls";
import * as THREE from "three";
import reportWebVitals from './reportWebVitals';
import Button from 'react-bootstrap/Button';
import Dropdown from 'react-bootstrap/Dropdown';
import DropdownButton from 'react-bootstrap/DropdownButton';
import ButtonGroup from 'react-bootstrap/ButtonGroup';
import Container from 'react-bootstrap/Container';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import { faLocationCrosshairs } from '@fortawesome/free-solid-svg-icons'
import { faCarBattery } from '@fortawesome/free-solid-svg-icons'
import { faSignal } from '@fortawesome/free-solid-svg-icons'
import { faPowerOff } from '@fortawesome/free-solid-svg-icons'

// import css styles
import './index.css';

// import custom components
import Joy from './joy.js';
import WebRTCComponent from './connection.js';



const CameraController = () => {
  const { camera, gl } = useThree();
  useEffect(
    () => {
      const controls = new OrbitControls(camera, gl.domElement);
      controls.minDistance = 3;
      controls.maxDistance = 20;
      return () => {
        controls.dispose();
      };
    },
    [camera, gl]
    );
    return null;
  };
  
function App() {
  const [xPos, setXPos] = useState(0);
  const [yPos, setYPos] = useState(0);
  
  function handleMove(event) {
    setXPos(event.x);
    setYPos(event.y);
  }
  
  function handleStop(event) {
    setXPos(0);
    setYPos(0);
  }

  return ( 
    <>
      <head>
        <link
          rel="stylesheet"
          href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css"
          integrity="sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN"
          crossorigin="anonymous"
        />

      </head>
      <body>
        <div class="fixed-top">
            <Row style={{padding: '4px'}}>
              <Col sx={12}>
                <Row>
                  <Col>
                    <Row>
                      <Col>
                        <Dropdown>
                          <Dropdown.Toggle variant="outline-light" id="dropdown-basic">
                            Mode Selection 
                          </Dropdown.Toggle>

                          <Dropdown.Menu>
                            <Dropdown.Item >Keep Location</Dropdown.Item>
                            <Dropdown.Item >Manual Drive</Dropdown.Item>
                            <Dropdown.Item >Path Follow</Dropdown.Item>
                            <Dropdown.Item >Autonomous Explore</Dropdown.Item>
                          </Dropdown.Menu>
                        </Dropdown>
                      </Col>
                      <Col>
                        <ButtonGroup aria-label="Speed Selection">
                          <Button variant="outline-light">Slow</Button>
                          <Button variant="success">Normal</Button>
                          <Button variant="outline-light">Fast</Button>
                        </ButtonGroup>
                      </Col>
                    </Row>
                                
                  </Col>
                  <Col xs={3}>
                    <Row style={{padding: '0px'}}>
                      <Col style={{padding: '0px 2px'}}>
                        <Button variant="outline-light" style={{width:'100%'}}>
                          <FontAwesomeIcon icon={faLocationCrosshairs}/>
                        </Button>
                      </Col>
                      <Col style={{padding: '0px 2px'}}>
                        <Button variant="outline-light" style={{width:'100%'}}>
                          <FontAwesomeIcon icon={faCarBattery}/>
                        </Button>
                      </Col>
                      <Col style={{padding: '0px 2px'}}>
                        <Button variant="outline-light" style={{width:'100%'}}>
                          <FontAwesomeIcon icon={faSignal}/>
                        </Button>
                      </Col>
                      <Col style={{padding: '0px 2px'}}>
                        <Button variant="outline-light" style={{width:'100%'}}>
                          <FontAwesomeIcon icon={faPowerOff}/>
                        </Button>
                      </Col>
                    </Row>
                  </Col>
                  
                </Row>
              </Col>
              <Col xs={2} style={{padding: '0px 15px'}}>
                <Button variant="danger" style={{width: '100%'}}>
                  ESTOP
                </Button>
              </Col>
              

          {/* <Col>
          
          </Col>
              <Col>
              </Col>
              <Col>
                <Button variant="info">Battery</Button>{' '}
              </Col>
          */}
          </Row>
        </div>
    
        <div id="fullscreen-container">
          <Canvas>
      <color attach="background" args={['#202020']} />
        <CameraController />
        <ambientLight intensity={1} />
        <directionalLight color="red" position={[0, 0, 5]} />
        <mesh>
          <boxGeometry args={[1, 1, 1]} />
          <meshStandardMaterial color="orange" />
        </mesh>
        
        <mesh position={[0, 0, 0]}>
          <planeGeometry args={[100, 100]} />
          <shadowMaterial color={"black"} opacity={1} />
          <planeGeometry rotateX={(-Math.PI / 2)}/>
        </mesh>

        <gridHelper args={[200, 200]} position={[0,0,0]} opacity={1} >
      
        </gridHelper>
      </Canvas>
          {/* <WebRTCComponent joyXY={[xPos, yPos]} /> */}
          <Joy handleMove={handleMove} handleStop={handleStop} />
        </div>
      </body>
    </>
  )
};


createRoot(document.getElementById('root')).render(<App />)
reportWebVitals();
