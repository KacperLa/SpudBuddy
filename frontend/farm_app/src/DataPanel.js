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
        <tbody>
          <tr>
            <td className='data-cell'>Download Moisture Data</td>
            <td className='data-cell'>
              <Button
                variant='light'
                onClick={() => console.log("Downloading Moisture Data")}
              >
                Download
              </Button>
            </td>
          </tr>
          <tr>
            <td className='data-cell'>Download Plant Data</td>
            <td className='data-cell'>
              <Button
                variant='light'
                onClick={() => console.log("Downloading Plant Data")}
              >
                Download
              </Button>
            </td>
          </tr>  
        </tbody>
      </table>
      </div>
    );
};

export default DataPanel;







