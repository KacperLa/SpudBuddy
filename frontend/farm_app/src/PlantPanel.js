import React, { useState, useEffect } from 'react';
import Button from 'react-bootstrap/Button';
import ButtonGroup from 'react-bootstrap/ButtonGroup';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';

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
        </Row>
        <Row>
          <Col xs={6}>
            Y:{y}
          </Col>  
        </Row>  
      </Button>
    );
};

function PlantPanel(props) {

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
      <table>
        <thead>
          <tr>
            <th scope='col'>Plant</th>
            <th scope='col'> Plant </th>
            <th scope='col'> Sense </th>
            <th scope='col'> Water </th>
          </tr>
        </thead>
        <tbody>
          {props.plantData != null ? props.plantData.map((plant, index) => {
            return (
              <tr key={index}>
                <td>{plant.name}</td>
                <td><LocationButton x={plant.location[0]} y={plant.location[1]} setDesiredPos={props.setDesiredPos}/></td>
                <td><LocationButton x={plant.sense[0]} y={plant.sense[1]} setDesiredPos={props.setDesiredPos}/></td>
                <td><LocationButton x={plant.water[0]} y={plant.water[1]} setDesiredPos={props.setDesiredPos}/></td>
                <td>
                  <Button size="sm" variant="outline-light">Read</Button>
                </td>
              </tr>
            );
          }
          ) : null}
        </tbody>
      </table>

      </div>
    );
};

export default PlantPanel;







