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
import { faSignal } from '@fortawesome/free-solid-svg-icons'
import { faEye } from '@fortawesome/free-solid-svg-icons'

// import css styles
import './index.css';

// import custom components
import Joy from './joy.js';
import ConnectivityComponent from './connection.js';
import ThreeView from './threeView.js';
import OptionsView from './optionsView.js';

function App() {
  // define xPos and yPos as a array of two elements
  const [robotPos, setPos] = useState([0, 0]);
  const [zoom, setZoom] = useState(false);
  const [selectedMode, setSelectedMode] = useState('Mode Selection');


  function handleMove(event) {
    setPos([event.x, event.y]);
  }
  
  function handleStop(event) {
    setPos([0, 0]);
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
                        <Button size="lg" variant="outline-light" style={{width:'100%'}}>
                          <FontAwesomeIcon icon={faEye}/>
                        </Button>
                      </Col>
                      <Col style={{padding: '0px 2px'}}>
                        <Button size="lg" variant="outline-light" style={{width:'100%'}}>
                          <FontAwesomeIcon icon={faLocationCrosshairs}/>
                        </Button>
                      </Col>
                      <Col style={{padding: '0px 2px'}}>
                        {/* <ConnectivityComponent joyXY={pos} /> */}
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
          <ThreeView />
        </div>
        <OptionsView
          position={{top: '80px', left: '30px'}}
          content={
            <ButtonGroup aria-label="Speed Selection">
                <Button size="lg" variant="outline-light">Plants</Button>
                <Button size="lg" variant="outline-light">Readings</Button>
            </ButtonGroup>
          }
        />
        <OptionsView
          position={{bottom: '50px', left: '50px'}}
          content={
            <div
              style={
                {
                  color: 'white',
                }
              }
            >
              <Col style={{padding: '10px 30px'}}>
                <Row>
                  Current Mode: {robotPos[0]} X {robotPos[1]} Y
                </Row>
                <Row>
                  Current Position:
                </Row>
              </Col>
            </div>
          }
        />

        <Joy handleMove={handleMove} handleStop={handleStop} />

    </>
  )
};


createRoot(document.getElementById('root')).render(<App />)
reportWebVitals();
