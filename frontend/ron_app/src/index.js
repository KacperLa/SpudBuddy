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
import ThreeView from './threeView.js';
import FloatingPictureInPicture from './videoView.js'

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
  // define xPos and yPos as a array of two elements
  const [pos, setPos] = useState([0, 0]);
  const [zoom, setZoom] = useState(false);


  const videoRef = useRef(null);
  const threeRef = useRef(null);

  function handleMove(event) {
    setPos([event.x, event.y]);
  }
  
  function handleStop(event) {
    setPos([0, 0]);
  }

  function swapViews() {
    // get by id the fullscreen-container
    const container = document.getElementById('fullscreen-container');
    // get by id the pip
    const pip = document.getElementById('pip');
    // if the container is not null, then swap the views
    if (container) {
      // get the current display style of the container
      const display = container.style.display;
      // if the display is none, then set the display to block
      if (display === 'none') {
        container.style.display = 'block';
        pip.style.display = 'none';
      } else {
        container.style.display = 'none';
        pip.style.display = 'block';
      }
    }

  }


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
                        <Button size="lg" onClick={swapViews} variant="outline-light" style={{width:'100%'}}>
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
                        <WebRTCComponent joyXY={pos} zoom_level={zoom} video_ref={videoRef} />
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
        

        <div id="fullscreen-container">
            <ThreeView />
        </div>
          <Joy handleMove={handleMove} handleStop={handleStop} />
          <FloatingPictureInPicture id="pip" setZoom={setZoom} content={
            <video ref={videoRef} autoPlay style={{ margin: '0px', padding: '0px', width: '100%', borderRadius: '15px' }} />
          } />
    </>
  )
};


createRoot(document.getElementById('root')).render(<App />)
reportWebVitals();
