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

// import arrows from fontawesome
import { faArrowUp, faArrowDown, faArrowLeft, faArrowRight } from '@fortawesome/free-solid-svg-icons'

// import css styles
import './index.css';

// import custom components
import Joy from './joy.js';
import ConnectivityComponent from './connection.js';
import ThreeView from './threeView.js';
import OptionsView from './optionsView.js';

function App() {
  // define xPos and yPos as a array of two elements
  const [robotPos, setRobotPos] = useState([null, null, null]);
  const [desiredPos, setDesiredPos] = useState([0, 0, 0]);
  const [zoom, setZoom] = useState(false);
  const [selectedMode, setSelectedMode] = useState('Mode Selection');


  function handleMove(event) {
    setDesiredPos([event.x, event.y]);
  }
  
  function handleStop(event) {
    setDesiredPos([0, 0]);
  }

  function updateDesiredPos() {
    console.log("Setting desired position to:", document.getElementById('desiredX').value, document.getElementById('desiredY').value, document.getElementById('desiredZ').value);
    setDesiredPos([document.getElementById('desiredX').value, document.getElementById('desiredY').value, document.getElementById('desiredZ').value]);
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
                        <ConnectivityComponent setRobotPos={setRobotPos} desiredPos={desiredPos}/>
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
              <table
                style={{
                  color: 'white',
                  width: '200px',
                  height: '100px',
                  margin: '10px',
                  padding: '10px',
                  textAlign: 'left',
                }}
              >
                <thead>
                  <tr>
                    <th scope='col' style={{ width: '60px' }}>Position</th>
                    <th scope='col' style={{ width: '30px' }}> X </th>
                    <th scope='col' style={{ width: '30px' }}> Y </th>
                    <th scope='col' style={{ width: '30px' }}> Z </th>
                  </tr>
                </thead>
                <tr>
                  <td scope='row'>Current:</td>
                  <td>{robotPos[0]}</td>
                  <td>{robotPos[1]}</td>
                  <td>{robotPos[2]}</td>
                </tr>
                <tr>
                  <td scope='row'>Desired:</td>
                  <td>{robotPos[0]}</td>
                  <td>{robotPos[1]}</td>
                  <td>{robotPos[2]}</td>
                </tr>
              </table>
          }
        />
        
        <OptionsView
          position={{bottom: '50px', right: '50px'}}
          content={
            <div
              style={{
                display: 'flex',
                flexDirection: 'column',
                justifyContent: 'center',
                alignItems: 'center',
                color: 'white',
              }}
            >
              <div
                style={{
                  margin: '10px',
                  width: '90%',
                  textAlign: 'center',
                }}
              >
                Reletive Positioning
              </div>
              <ButtonGroup
                aria-label="Speed Selection"
              >
                <Button size="sm" variant="outline-light">1</Button>
                <Button size="sm" variant="outline-light">10</Button>
                <Button size="sm" variant="outline-light">100</Button>
                <Button size="sm" variant="outline-light">1000</Button>
              </ButtonGroup>
              <table
                style={{
                  color: 'white',
                  width: '180px',
                  height: '100px',
                  margin: '10px',
                  padding: '10px',
                  textAlign: 'left',
                }}
              >
                <tr>
                  <td></td>
                  <td style={{ width: '30%' }}>
                    <Button size="lg" variant="outline-light">
                      <FontAwesomeIcon icon={faArrowUp}/>
                    </Button>                  </td>
                  <td></td>
                </tr>
                <tr>
                  <td style={{ width: '30%' }}>
                    <Button size="lg" variant="outline-light">
                      <FontAwesomeIcon icon={faArrowLeft}/>
                    </Button>
                  </td>
                  <td></td>
                  <td style={{ width: '30%' }}>
                    <Button size="lg" variant="outline-light">
                      <FontAwesomeIcon icon={faArrowRight}/>
                    </Button>
                  </td>
                </tr>
                <tr>
                  <td></td>
                  <td style={{ width: '30%' }}>
                    <Button size="lg" variant="outline-light">
                      <FontAwesomeIcon icon={faArrowDown}/>
                    </Button>
                  </td>
                  <td></td>
                </tr>
              </table>

              <div
                style={{
                  width: '90%',
                  textAlign: 'center',
                }}
              >
                Absolute Positioning
              </div>
              <table
                style={{
                  color: 'white',
                  width: '200px',
                  height: '50px',
                  margin: '10px',
                  padding: '10px',
                  textAlign: 'center',
                }}
              >
                <tr>
                  <td>X</td>
                  <td>Y</td>
                  <td>Z</td>
                </tr>
                <tr>
                  <td>
                    <input id='desiredX' style={{ width: '50px' }} type="text" defaultValue={'0'}/>
                  </td>
                  <td>
                    <input id='desiredY' style={{ width: '50px' }} type="text" defaultValue={'0'}/>
                  </td>
                  <td>
                    <input id='desiredZ' style={{ width: '50px' }} type="text" defaultValue={'0'}/>
                  </td>
                </tr>
              </table>
              <Button
                style={{
                  margin: '10px',
                  padding: '10px',
                  width: '90%',
                }}
                variant="outline-light"
                onClick={updateDesiredPos}
              >
                Go to Location
              </Button>
            </div>
          }
        />
    </>
  )
};


createRoot(document.getElementById('root')).render(<App />)
reportWebVitals();
