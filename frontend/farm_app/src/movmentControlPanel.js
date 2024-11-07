import React, { useState, useEffect } from 'react';
import Button from 'react-bootstrap/Button';
import ButtonGroup from 'react-bootstrap/ButtonGroup';
import './movementControlPanel.css';


import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import { faArrowUp, faArrowDown, faArrowLeft, faArrowRight, faHome, faFaucetDrip, faArrowUpFromGroundWater} from '@fortawesome/free-solid-svg-icons'

const RelativeMovementControlPanel = (props) => {
  const [movementIncrament, setMovementIncrament] = useState(1);
  
  function setRelativeMovement(x, y, z) {
    props.setDesiredPos([props.desiredPos[0] + x, props.desiredPos[1] + y, 0, 0]);
    props.setRobotCmd([1, props.desiredPos[0] + x, props.desiredPos[1] + y, 0, 0]);
  }

  return(
    <div
      style={{
        display: 'flex',
        width: '100%',
        padding: '0px',
        margin: '0px',
        flexDirection: 'column',
        justifyContent: 'center',
        alignItems: 'center',
      }}
    >
      <table
        style={{
          color: 'white',
          width: '100%',
          margin: '0px 0px',
          padding: '0px',
          boxSizing: 'content-box',  /* Include padding in the element's total width and height */
          textAlign: 'center',
          borderWidth: '0px',
          borderSpacing: '0px',
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
          </tr>
        </tbody>
      </table>
      
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
          margin: '0px',
          marginTop: '10px',
          padding: '0px',
          paddingTop: '0px',
          width: '100%',
        }}
      >
        <Button onClick={() => setMovementIncrament(1)} size="sm" variant={movementIncrament === 1 ? "light" : "outline-light"}>1</Button>
        <Button onClick={() => setMovementIncrament(10)} size="sm"variant={movementIncrament === 10 ? "light" : "outline-light"}>10</Button>
        <Button onClick={() => setMovementIncrament(100)} size="sm" variant={movementIncrament === 100 ? "light" : "outline-light"}>100</Button>
        <Button onClick={() => setMovementIncrament(1000)} size="sm" variant={movementIncrament === 1000 ? "light" : "outline-light"}>1000</Button>
      </ButtonGroup>
    </div>
  );
};

const AbsoluteMovementControlPanel = (props) => {
  useEffect(() => {
    document.getElementById('desiredX').value = props.desiredPos[0];
    document.getElementById('desiredY').value = props.desiredPos[1];
  }
  , [props.desiredPos]
  );

  return(
    <div
      style={{
        display: 'flex',
        width: '100%',
        padding: '0px',
        margin: '0px',
        flexDirection: 'column',
        justifyContent: 'center',
        alignItems: 'center',
      }}
    >
      <table
        style={{
          color: 'white',
          width: '100%',
          margin: '0px',
          padding: '0px',
          textAlign: 'center',
        }}
      >
      <thead>
        <tr>
          <th>
            X:
          </th>
          <th>
            Y:
          </th>
          </tr>
      </thead>
      <tbody>
        <tr>
          <td>
            <input style={{width: '100%'}} id='desiredX' type="text" defaultValue={props.desiredPos[0]}/>
          </td>
          <td>
            <input style={{width: '100%'}} id='desiredY' type="text" defaultValue={props.desiredPos[1]}/>
          </td>
        </tr>
      </tbody>
    </table>
    <Button
      style={{
        width: '100%',
        margin: '10px',
      }}
      size="lg"
      variant="outline-light"
      onClick={() => props.setRobotCmd([1, document.getElementById('desiredX').value, document.getElementById('desiredY').value, 0, 0]) 
      }
    >
      Go To Location
    </Button>
</div>
  );
};

function MovementControlPanel(props) {
  const [movementPanel, setMovementPanel] = useState('relative');

    return (
        <div
              style={{
                display: 'flex',
                width: '225px',
                padding: '0px',
                margin: '0px',
                boxSizing: 'border-box',  /* Include padding in the element's total width and height */
                flexDirection: 'column',
                justifyContent: 'center',
                alignItems: 'center',
                color: 'white',
              }}
            >
              <div
                style={{
                  margin: '10px',
                  width: '100%',
                  textAlign: 'center',
                }}
              >
                Positioning
              </div>

              <ButtonGroup
                aria-label="Positioning"
                style={{
                  margin: '0px 0px',
                  padding: '0px 10px',
                  width: '100%',
                }}
              >
                <Button  variant={movementPanel === "relative" ? "light" : "outline-light"} onClick={() => setMovementPanel('relative')}>Relative</Button>
                <Button  variant={movementPanel === "absolute" ? "light" : "outline-light"} onClick={() => setMovementPanel('absolute')}>Absolute</Button>
              </ButtonGroup>
              
              <div className="panel-container">
                {movementPanel === 'relative' && <RelativeMovementControlPanel desiredPos={props.desiredPos} setDesiredPos={props.setDesiredPos} setRobotCmd={props.setRobotCmd}/>}
                {movementPanel === 'absolute' && <AbsoluteMovementControlPanel desiredPos={props.desiredPos} setDesiredPos={props.setDesiredPos} setRobotCmd={props.setRobotCmd}/>}
              </div>

              <div
                style={{
                  margin: '0px',
                  padding: '10px',
                  width: '100%',
                  textAlign: 'center',
                }}
              >
                <div>
                  Actions
                </div>
                <table
                  style={{
                    color: 'white',
                    width: '100%',
                    margin: '0px 0px',
                    padding: '0px 0px',
                    boxSizing: 'border-box',  /* Include padding in the element's total width and height */
                    textAlign: 'center',
                  }}
                >
                  <tbody>
                    <tr>
                      <td  className="table-cell">
                        <Button
                          className="full-size-button"
                          size="lg"
                          variant="outline-light"
                        >
                        </Button>
                      </td>
                      <td className="table-cell">
                        <Button
                          className="full-size-button"
                          size="lg"
                          variant="outline-light"
                        >
                        </Button>
                      </td>
                      {/* <td  className="table-cell">
                        <Button
                          className="full-size-button"
                          size="lg"
                          variant="outline-light"
                          onClick={() => props.setRobotCmd([3, 0, 0, 0, 0])}
                        >
                          <FontAwesomeIcon icon={faFaucetDrip}/>
                        </Button>
                      </td> */}
                      <td className="table-cell">
                        <Button
                          className="full-size-button"
                          size="lg"
                          variant="outline-light"
                          onClick={() => props.setRobotCmd([3, 0, 0, 0, 0])}
                        >
                          <FontAwesomeIcon icon={faHome}/>
                        </Button>
                      </td>
                    </tr>
                  </tbody>
                </table>
              </div>
              
            </div>
    );
};

export default MovementControlPanel;