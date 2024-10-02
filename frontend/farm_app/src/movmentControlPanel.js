import React, { useState, useEffect } from 'react';
import Button from 'react-bootstrap/Button';
import ButtonGroup from 'react-bootstrap/ButtonGroup';

import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import { faArrowUp, faArrowDown, faArrowLeft, faArrowRight, faHome} from '@fortawesome/free-solid-svg-icons'

function MovementControlPanel(props) {
    const [movementIncrament, setMovementIncrament] = useState(1);

    function setRelativeMovement(x, y, z) {
        props.setDesiredPos([props.desiredPos[0] + x, props.desiredPos[1] + y, props.desiredPos[2] + z]);
        // figure out how to send this to the robot
    }

    useEffect(() => {
        document.getElementById('desiredX').value = props.desiredPos[0];
        document.getElementById('desiredY').value = props.desiredPos[1];
        document.getElementById('desiredZ').value = props.desiredPos[2];
    }
    , [props.desiredPos]
    );


    return (
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
              <div 
                style={{
                    fontSize: '.75em',
                    height: '10px',
                    textAlign: 'center',
                    padding: '0px',
                    margin: '0px', 
                }}
              >
                Movment Incrament (mm)
              </div>
              <ButtonGroup
                aria-label="Movment Incrament"
                style={{
                  margin: '10px',
                  padding: '10px',
                  paddingTop: '0px',
                  width: '90%',
                }}
              >
                <Button onClick={() => setMovementIncrament(1)} size="sm" variant={movementIncrament === 1 ? "light" : "outline-light"}>1</Button>
                <Button onClick={() => setMovementIncrament(10)} size="sm"variant={movementIncrament === 10 ? "light" : "outline-light"}>10</Button>
                <Button onClick={() => setMovementIncrament(100)} size="sm" variant={movementIncrament === 100 ? "light" : "outline-light"}>100</Button>
                <Button onClick={() => setMovementIncrament(1000)} size="sm" variant={movementIncrament === 1000 ? "light" : "outline-light"}>1000</Button>
              </ButtonGroup>
              <table
                style={{
                  color: 'white',
                  width: '180px',
                  height: '180px',
                  margin: '10px',
                  padding: '10px',
                  textAlign: 'center',
                }}
              >
                <tbody>
                  <tr>
                    <td  className="table-cell"></td>
                    <td  className="table-cell">
                      <Button
                        className="full-size-button"
                        size="lg"
                        variant="outline-light"
                        onClick={() => setRelativeMovement(0, movementIncrament, 0)}
                      >
                        <FontAwesomeIcon icon={faArrowUp}/>
                      </Button>
                    </td>
                    <td  className="table-cell"></td>
                    <td  className="table-cell">
                      <Button
                        className="full-size-button"
                        size="lg"
                        variant="outline-light"
                        onClick={() => setRelativeMovement(0, 0, movementIncrament)}
                      >
                        <FontAwesomeIcon icon={faArrowUp}/>
                      </Button>
                    </td>
                  </tr>
                  <tr>
                    <td  className="table-cell">
                      <Button
                        className="full-size-button"
                        size="lg"
                        variant="outline-light"
                        onClick={() => setRelativeMovement(-movementIncrament, 0, 0)}
                      >
                        <FontAwesomeIcon icon={faArrowLeft}/>
                      </Button>
                    </td>
                    <td  className="table-cell">
                      <Button
                        className="full-size-button"
                        size="lg"
                        variant="outline-light"
                        onClick={ () => {
                                            props.setDesiredPos([3, 0, 0, 0, 0]);
                                }}
                      >
                        <FontAwesomeIcon icon={faHome}/>
                      </Button>
                    </td>
                    <td  className="table-cell">
                      <Button
                        className="full-size-button"
                        size="lg"
                        variant="outline-light"
                        onClick={() => setRelativeMovement(movementIncrament, 0, 0)}>
                        <FontAwesomeIcon icon={faArrowRight}/>
                      </Button>
                    </td>
                    <td  className="table-cell">Z</td>
                  </tr>
                  <tr>
                    <td  className="table-cell"></td>
                    <td  className="table-cell">
                      <Button
                        className="full-size-button"
                        size="lg"
                        variant="outline-light"
                        onClick={() => setRelativeMovement(0, -movementIncrament, 0)}
                      >
                        <FontAwesomeIcon icon={faArrowDown}/>
                      </Button>
                    </td>
                    <td  className="table-cell"></td>
                    <td  className="table-cell">
                      <Button
                        className="full-size-button"
                        size="lg"
                        variant="outline-light"
                        onClick={() => setRelativeMovement(0, 0, -movementIncrament)}
                      >
                        <FontAwesomeIcon icon={faArrowDown}/>
                      </Button>
                    </td>

                  </tr>
                </tbody>
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
                <thead>
                </thead>
                <tbody>
                  <tr>
                    <td>X</td>
                    <td>Y</td>
                    <td>Z</td>
                  </tr>
                  <tr>
                    <td>
                      <input id='desiredX' style={{ width: '50px' }} type="text" defaultValue={props.desiredPos[0]}/>
                    </td>
                    <td>
                      <input id='desiredY' style={{ width: '50px' }} type="text" defaultValue={props.desiredPos[1]}/>
                    </td>
                    <td>
                      <input id='desiredZ' style={{ width: '50px' }} type="text" defaultValue={props.desiredPos[2]}/>
                    </td>
                  </tr>
                </tbody>
              </table>
              <Button
                style={{
                  margin: '10px',
                  padding: '10px',
                  width: '90%',
                }}
                variant="outline-light"
                onClick={() => props.setDesiredPos([document.getElementById('desiredX').value, document.getElementById('desiredY').value, document.getElementById('desiredZ').value]) 
                }
              >
                Go to Location
              </Button>
            </div>
    );
};

export default MovementControlPanel;