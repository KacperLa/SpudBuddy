import React, { useState, useEffect } from 'react';
import Button from 'react-bootstrap/Button';
import ButtonGroup from 'react-bootstrap/ButtonGroup';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';

import Accordion from 'react-bootstrap/Accordion';


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
   return (
      <div
        style={{
          display: 'flex',
          flexDirection: 'column',
          justifyContent: 'left',
          alignItems: 'left',
          color: 'white',
          passing: '10px',
          margin: '10px',
        }}
      >
      <Accordion defaultActiveKey="0">
        {Object.keys(props.plantData).map((plant, index) => (
          <Accordion.Item eventKey={index} key={index}>
            <Accordion.Header>
              {plant}
            </Accordion.Header>
            <Accordion.Body>
                <table>
                    <tbody>
                        <tr>
                            <td>Location</td>
                            <td
                                style={{
                                    textAlign: 'right',
                                }}
                            >
                          <LocationButton x={props.plantData[plant].location[0]} y={props.plantData[plant].location[1]} setDesiredPos={props.setDesiredPos}/>
                          </td>
                        </tr> 
                        <tr>
                            <td>Sense</td>
                            <td
                                style={{
                                    textAlign: 'right',
                                }}
                            >
                                <LocationButton x={props.plantData[plant].sense[0]} y={props.plantData[plant].sense[1]} setDesiredPos={props.setDesiredPos}/>
                            </td>
                        </tr>
                        <tr>
                            <td>Water</td>
                            <td
                                style={{
                                    textAlign: 'right',
                                }}
                            >
                                <LocationButton x={props.plantData[plant].water[0]} y={props.plantData[plant].water[1]} setDesiredPos={props.setDesiredPos}/>
                            </td>
                        </tr>
                        <tr>
                            <td>Water Amount</td>
                            <td
                                style={{
                                    textAlign: 'right',
                                }}
                            >{props.plantData[plant].ml_response}</td>
                        </tr>
                        <tr>
                            <td>Moisture Threshold</td>
                            <td
                                style={{
                                    textAlign: 'right',
                                }}
                            >
                                {props.plantData[plant].moisture_threshhold}
                            </td>
                        </tr>
                    </tbody>
                </table>
                <Button size="sm" variant="outline-light">Probe</Button>
                <Button size="sm" variant="outline-light">Edit</Button>

            </Accordion.Body>
          </Accordion.Item>
        ))}
        
      
      </Accordion>
      </div>
    );
});

export default PlantPanel;







