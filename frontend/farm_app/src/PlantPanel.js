import React, { useState, useEffect } from 'react';
import Button from 'react-bootstrap/Button';
import ButtonGroup from 'react-bootstrap/ButtonGroup';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';

import Accordion from 'react-bootstrap/Accordion';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome'
import { faLocation } from '@fortawesome/free-solid-svg-icons'


import './PlantPanel.css';

const LocationButton = ({x, y, setDesiredPos}) => {
    return (
      <Button
        className='location-button'
        onClick={() => setDesiredPos([x, y, 0])}
      >
        <Row>
          <Col xs={6}>
            X:{x}
          </Col>
          <Col xs={6}>
            Y:{y}
          </Col>  
        </Row>  
      </Button>
    );
};



const PlantPanel = React.memo((props) => {
  function fillInLocation(x, y) {
    document.getElementById(x).value = props.robotPos[0];
    document.getElementById(y).value = props.robotPos[1];
  }

  function addPlant(name, ml_response, moisture_threshhold, sense_x, sense_y, water_x, water_y) {
    // Check that all the fields are filled in
    if (name === "" || ml_response === "" || moisture_threshhold === "" || sense_x === "" || sense_y === "" || water_x === "" || water_y === "") {
      alert("Please fill in all fields");
      return;
    }

    // Check that all the fields are numbers
    if (isNaN(ml_response) ||
        isNaN(moisture_threshhold) ||
        isNaN(sense_x) || isNaN(sense_y) ||
        isNaN(water_x) || isNaN(water_y))
    {
      alert("Paramater fields must be numbers");
      return;
    }

    // Check that all locations are within the farm
    if (sense_x < 0 || sense_x >= props.farmData.gantry_size[0] ||
        sense_y < 0 || sense_y >= props.farmData.gantry_size[1] ||
        water_x < 0 || water_x >= props.farmData.gantry_size[0] ||
        water_y < 0 || water_y >= props.farmData.gantry_size[1])
    {
      alert("All locations must be within the farm");
      return;
    }

    // first byte is the plant message is 0x03
    // first 9 bytes is the plant name
    // byte 11-12 is the x position as uint16
    // byte 13-14 is the y position as uint16
    // byte 15-16 is the sense x position as uint16
    // byte 17-18 is the sense y position as uint16
    // byte 19 is the ml to water  as uint8
    // byte 20 is the moisture threshold as uint8 
  
    let data = new Uint8Array(20);
    data[0] = 0x03;
    let encoder = new TextEncoder();
    let nameArray = encoder.encode(name);
    for (let i = 0; i < 9; i++) {
      data[i+1] = nameArray[i];
    }

    data[10] = water_x & 0xff;
    data[11] = water_x >> 8;
    data[12] = water_y & 0xFF;
    data[13] = water_y >> 8;
    data[14] = sense_x & 0xFF;
    data[15] = sense_x >> 8;
    data[16] = sense_y & 0xFF;
    data[17] = sense_y >> 8;
    
    data[18] = ml_response;
    data[19] = moisture_threshhold;

    console.log(data);
    props.sendData(data);
  }

   return (
    <div
      className="scrollable-panel"
      style={{
        display: 'flex',
        width: '300px',
        flexDirection: 'column',
        justifyContent: 'left',
        alignItems: 'left',
        color: 'white',
        passing: '10px',
        margin: '10px',
        height: '400px', // Set a fixed height
        overflowY: 'auto', // Make the panel scrollable vertically
        overflowX: 'hidden',
      }}
    >
      <Accordion defaultActiveKey="0">
        {Object.keys(props.farmData.plants).map((plant, index) => (
          <Accordion.Item eventKey={index} key={index}>
            <Accordion.Header>
              {plant}
            </Accordion.Header>
            <Accordion.Body>
                <table>
                    <tbody>
                        <tr>
                            <td>Sense</td>
                            <td
                                style={{
                                    textAlign: 'right',
                                }}
                            >
                                <LocationButton x={props.farmData.plants[plant].sense[0]} y={props.farmData.plants[plant].sense[1]} setDesiredPos={props.setDesiredPos}/>
                            </td>
                        </tr>
                        <tr>
                            <td>Water</td>
                            <td
                                style={{
                                    textAlign: 'right',
                                }}
                            >
                                <LocationButton x={props.farmData.plants[plant].water[0]} y={props.farmData.plants[plant].water[1]} setDesiredPos={props.setDesiredPos}/>
                            </td>
                        </tr>
                        <tr>
                            <td>Water Amount</td>
                            <td
                                style={{
                                    textAlign: 'right',
                                }}
                            >{props.farmData.plants[plant].ml_response}</td>
                        </tr>
                        <tr>
                            <td>Moisture Threshold</td>
                            <td
                                style={{
                                    textAlign: 'right',
                                }}
                            >
                                {props.farmData.plants[plant].moisture_threshhold}
                            </td>
                        </tr>
                    </tbody>
                </table>
                <Button size="sm" variant="outline-light">Probe</Button>
                <Button size="sm" variant="outline-light">Edit</Button>

            </Accordion.Body>
          </Accordion.Item>
        ))}
        
      
        <Accordion.Item>
        <Accordion.Header>
          Add Mission
        </Accordion.Header>
        <Accordion.Body>
          <table
            style={{
              width: '100%',
            }}
          >
            <tbody>
              <tr>
                <th className='data-cell'>Plant Name:</th>
                <td className='data-cell-right'>
                  <input style={{ width: '110px' }} id="plant_name" type="text" placeholder="plant name"/>
                </td>
              </tr>
              <tr>
                <th className='data-cell'>Location</th>
              </tr>
              <tr>
                <th className='data-cell'>Sense</th>
                <td className='data-cell-right'>
                  <div className="input-container">
                    <input style={{ width: '60px' }} id="sense_x" type="text" placeholder="X"/>
                    <input style={{ width: '60px' }} id="sense_y" type="text" placeholder="Y"/>
                    <Button
                      size="sm"
                      variant="outline-light"
                      onClick={() => fillInLocation("sense_x", "sense_y")}
                    >
                      <FontAwesomeIcon icon={faLocation} />
                    </Button>
                  </div>
                </td>
              </tr>
              <tr>
                <th className='data-cell'>Water</th>
                <td className='data-cell-right'>
                  <div className="input-container">
                    <input style={{ width: '60px' }} id="water_x" type="text" placeholder="X"/>
                    <input style={{ width: '60px' }} id="water_y" type="text" placeholder="Y"/>
                    <Button
                      size="sm"
                      variant="outline-light"
                      onClick={() => fillInLocation("water_x", "water_y")}
                    >
                      <FontAwesomeIcon icon={faLocation} />
                    </Button>
                  </div>
                </td>
              </tr>
              <tr>
                <th className='data-cell'>Water Amount:</th>
                <td className='data-cell-right'>
                  <input style={{ width: '110px' }} id="water_amount" type="text" placeholder="ml"/>
                </td>
              </tr>
              <tr>
                <th className='data-cell'>Moisture Threshold:</th>
                <td className='data-cell-right'>
                  <input style={{ width: '100px' }} id="moisture_threshold" type="text" placeholder=".5"/>
                </td>
              </tr>
            </tbody>
          </table>
          
          <Button
            style={{
              width: '100%',
            }}
            variant="outline-light"
            onClick={() => addPlant(
              document.getElementById('plant_name').value,
              document.getElementById('water_amount').value,
              document.getElementById('moisture_threshold').value,
              document.getElementById('sense_x').value,
              document.getElementById('sense_y').value,
              document.getElementById('water_x').value,
              document.getElementById('water_y').value
            )}
          >
            Add Plant
          </Button>
          </Accordion.Body>
      </Accordion.Item>

      </Accordion>
      </div>
    );
});

export default PlantPanel;







