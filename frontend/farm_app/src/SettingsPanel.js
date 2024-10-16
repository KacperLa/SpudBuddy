import React, { useState } from 'react';
import Button from 'react-bootstrap/Button';

import Accordion from 'react-bootstrap/Accordion';

import './settingsPanel.css';

const addMission = (action, hour, minute, plants) => {
  console.log("Adding Mission: ", action, hour, minute, plants);
}

const SettingsPanel = React.memo((props) => {
  console.log("Settings Data: ", props.settingsData);
  
  const [checkedPlants, setCheckedPlants] = useState([]);

  const handleCheckboxChange = (event) => {
    const plant = event.target.value;
    if (event.target.checked) {
      setCheckedPlants([...checkedPlants, plant]);
    } else {
      setCheckedPlants(checkedPlants.filter((p) => p !== plant));
    }
  };
  
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
      {props.machineData.missions ? (
        props.machineData.missions.map((mission, index) => (
          <Accordion.Item eventKey={index} key={mission.mission_id}>
            <Accordion.Header>
              {mission.mission_name}
            </Accordion.Header>
            <Accordion.Body>
              <table
                style={{
                  width: '100%',
                }}
              >
                <tbody>
                  <tr>
                    <th className='data-cell'>Hour:</th>
                    <td className='data-cell-right'> {mission.time[0]} </td>
                  </tr>
                  <tr>
                    <th className='data-cell'>Minute:</th>
                    <td className='data-cell-right'> {mission.time[1]} </td>
                  </tr>
                  <tr>
                    <th className='data-cell'>Action:</th>
                    <td className='data-cell-right'> {mission.type} </td>
                  </tr>
                </tbody>
              </table>

              <table
                style={{
                  width: '100%',
                }}
              >
                <tbody>
                  <tr>
                    <th className='data-cell'>Plants:</th>
                  </tr>
                  {mission.locations.map((location, index) => (
                    <tr>
                      <td className='data-cell-right'>{location}</td>
                    </tr>
                ))}
                </tbody>
              </table>


              <Button
                style={{
                  width: '100%',
                }}
                variant="outline-light"
                onClick={() => props.robotCmd([5, mission.mission_id, 0, 0, 0])}
              >
                Run Mission
              </Button>
            </Accordion.Body>
          </Accordion.Item>
        ))
      ) : null}

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
                <th className='data-cell'>Mission Name:</th>
                <td className='data-cell-right'>
                  <input style={{ width: '120px' }} id="name" type="text" placeholder="mission_0"/>
                </td>
              </tr>
              <tr>
                <th className='data-cell'>Hour</th>
                <td className='data-cell-right'>
                  <input style={{ width: '120px' }} id="hour" type="text" placeholder="Hour"/>
                </td>
              </tr>
              <tr>
                <th className='data-cell'>Minute</th>
                <td className='data-cell-right'>
                  <input style={{ width: '120px' }} id="minute" type="text" placeholder="Minute"/>
                </td>
              </tr>
              <tr>
                <th className='data-cell'>Action</th>
                <td className='data-cell-right'>
                <select id="action" name="Action" style={{ width: '120px' }}>
                  <option value="water">Water</option>
                  <option value="sense">Sense</option>
                  <option value="visit">Visit</option>
                </select>                  
                </td>
              </tr>
            </tbody>
          </table>
          

          {props.machineData.plants ? (
            Object.keys(props.machineData.plants).map((plant, index) => (
              <div className="checkbox-container" key={index}>
                <input
                  type="checkbox"
                  id={`plant-${index}`}
                  name={plant}
                  value={plant}
                  onChange={handleCheckboxChange}
                />
                <label htmlFor={`plant-${index}`}>{plant}</label>
              </div>
            ))
          ) : null}

          <Button
            style={{
              width: '100%',
            }}
            variant="outline-light"
            onClick={() => addMission(
              document.getElementById('action').value,
              document.getElementById('hour').value,
              document.getElementById('minute').value,
              checkedPlants
            )}
          >
            Add Mission
          </Button>
          </Accordion.Body>
      </Accordion.Item>
    </Accordion>
    </div>
  );
});

export default SettingsPanel;







