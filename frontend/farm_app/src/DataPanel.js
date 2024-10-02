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

function DataPanel(props) {
    console.log("Plant Data: ", props.plantData);
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
            <th className='data-cell' scope='col'>Date</th>
            <th className='data-cell' scope='col'>Plant</th>
            <th className='data-cell' scope='col'>Moisture Reading</th>
            <th className='data-cell' scope='col'>Water Amount</th>
          </tr>
        </thead>
        <tbody>
          {props.plantData != null ? Object.keys(props.plantData).map((date, index) => (
            <>
              {Object.keys(props.plantData[date]).map((plant, index) => (
                  <tr key={index}>
                   <td className='data-cell'> {date} </td>
                   <td className='data-cell'> {plant} </td>
                   <td className='data-cell'> {props.plantData[date][plant][0]} </td>
                   <td className='data-cell'> {props.plantData[date][plant][0]} </td>
                  </tr>
              ))}
          </>
          )) : null}
        </tbody>
      </table>

      </div>
    );
};

export default DataPanel;







