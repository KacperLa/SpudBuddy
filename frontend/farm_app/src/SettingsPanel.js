import React, { useState, useEffect } from 'react';
import Button from 'react-bootstrap/Button';
import ButtonGroup from 'react-bootstrap/ButtonGroup';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';

import Accordion from 'react-bootstrap/Accordion';

import './PlantPanel.css';
import { ListGroup } from 'react-bootstrap';

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
        </Row>
        <Row>
          <Col xs={6}>
            Y:{y}
          </Col>  
        </Row>  
      </Button>
    );
};

const SettingsPanel = React.memo((props) => {
  console.log("Settings Data: ", props.settingsData);
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
      {props.settingsData ? (
        props.settingsData.map((mission, index) => (
          <Accordion.Item eventKey={index} key={mission.mission_id}>
            <Accordion.Header>
              {mission.mission_name}
            </Accordion.Header>
            <Accordion.Body>
              <table>
                <thead>
                  <tr>
                    <th className='data-cell' scope='col'>Hour</th>
                    <th className='data-cell' scope='col'>Minute</th>
                    <th className='data-cell' scope='col'>Action</th>
                  </tr>
                </thead>
                <tbody>
                  <tr>
                    <td className='data-cell'> {mission.time[0]} </td>
                    <td className='data-cell'> {mission.time[1]} </td>
                    <td className='data-cell'> {mission.type} </td>
                  </tr>
                </tbody>
              </table>

              {/* <ListGroup>
                {mission.locations.map((location, index) => (
                  <ListGroup.Item key={index}>
                    <LocationButton x={location[0]} y={location[1]} setDesiredPos={props.setDesiredPos}/>     
                  </ListGroup.Item>
                ))}
              </ListGroup> */}


              <Button
                variant="outline-light"
                onClick={() => props.robotCmd([5, mission.mission_id, 0, 0, 0])}
              >
                Run Mission
              </Button>
            </Accordion.Body>
          </Accordion.Item>
        ))
      ) : null}
    </Accordion>
    </div>
  );
});

export default SettingsPanel;







