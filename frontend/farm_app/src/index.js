import React, {useState, useEffect, useCallback} from 'react';

import { createRoot } from 'react-dom/client'
import reportWebVitals from './reportWebVitals';
import Button from 'react-bootstrap/Button';
import ButtonGroup from 'react-bootstrap/ButtonGroup';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';

// import json from './assets/sample_data.json';
import sample_data from './assets/sample_json.json';  


import './index.css';

import ConnectivityComponent from './connection.js';
import ThreeView from './threeView.js';
import OptionsView from './optionsView.js';
import MovementControlPanel from './movmentControlPanel.js';
import PlantPanel from './PlantPanel.js';
import DataPanel from './DataPanel.js';
import SettingsPanel from './SettingsPanel.js';

function App() {
  const [robotPos, setRobotPos] = useState([null, null, null, null, null]);
  const [desiredPos, setDesiredPos] = useState([0, 0, 0]);
  const [datatoSend, setDatatoSend] = useState(null);

  const [robotCmd, setRobotCmd] = useState([null, null, null, null, null]);

  const [farmSize, setFarmSize] = useState([1, 1]);
  const [farmData, setFarmData] = useState(sample_data); 

  const [plantView, setPlantView] = useState("plants");

  useEffect(() => {
    console.log("Farm Data Updated");
    console.log(farmData);
    if (farmData != null) {
      console.log(farmData.type);
      setFarmSize([farmData.gantry_size[0]/100, farmData.gantry_size[1]/100]);
      console.log("Setting farm size to:", farmData.gantry_size);
    }
  }, [farmData]);

  const memoizedSetRobotCmd = useCallback((cmd) => {
    setRobotCmd(cmd);
  }, []);    

  const memoizedSetDatatoSend = useCallback((data) => {
    if (data != null) {
      console.log("Setting Data to Send: ", data);
      setDatatoSend(data);
    }
  }, []);

  const deleteMission = useCallback((mission_id) => {
    setRobotCmd([8, mission_id, 0, 0, 0]);
  
    // Create a new array of missions without the deleted mission
    const newMissions = farmData.missions.filter((m) => m.mission_id !== mission_id);
  
    // Create a new farmData object with the updated missions
    const newFarmData = {
      ...farmData,
      missions: newMissions
    };
  
    // Update the state with the new farmData object
    setFarmData(newFarmData);
  }, [farmData]);

  const deletePlant = useCallback((plant) => {
    const plant_id = farmData.plants[plant].id;
    setRobotCmd([9, plant_id, 0, 0, 0]);
    
    console.log("Deleting Plant: ", plant);

    // Create a new array of plants without the deleted plant
    const newPlants = farmData.plants;
    delete newPlants[plant];
  
    // Create a new farmData object with the updated plants
    const newFarmData = {
      ...farmData,
      plants: newPlants
    };
  
    // Update the state with the new farmData object
    setFarmData(newFarmData);
  }, [farmData]);

  return ( 
    <>
        <div className="fixed-top" style={{zIndex: 10000}}>
          <Row style={{padding: '4px'}}>
              <Col sx={12}>
                <Row>
                  <Col>
                    <Row style={{padding: '0px 15px'}}>
                      <Col style={{padding: '2px 2px'}}>
                        <ConnectivityComponent setRobotPos={setRobotPos} datatoSend={datatoSend} robotCmd={robotCmd} setFarmData={setFarmData}/>
                      </Col>
                    </Row>
                  </Col>
                </Row>
              </Col>
              <Col xs={2} style={{padding: '0px 15px'}}>
                {/* link to issue page */}
                <Button size="lg" variant="danger" style={{width: '100%'}}
                href="https://github.com/KacperLa/SpudBuddy/issues"
                >
                  REPORT BUG
                </Button>
              </Col>
          </Row>
        </div>
        
        <div id="fullscreen-container" style={{color: 'black', background: 'black'}}>
          <ThreeView
            robotPos={robotPos}
            desiredPos={desiredPos}
            setDesiredPos={setDesiredPos}
            plantData={farmData.plants}
            farmSize={farmSize}
          />
        </div>

        <OptionsView
          position={{top: '8em', left: '30px'}}
          content={
            <div>

              <ButtonGroup>
                <Button variant={plantView === "plants" ? "light" : "outline-light"} onClick={() => setPlantView("plants")}>Plants</Button>
                <Button variant={plantView === "missions" ? "light" : "outline-light"} onClick={() => setPlantView("missions")}>Missions</Button>
              </ButtonGroup>

              {(plantView === "plants" && farmData.plants != null) && <PlantPanel sendData={memoizedSetDatatoSend} deletePlant={deletePlant} robotPos={robotPos} farmData={farmData} setDesiredPos={setDesiredPos}/>}
              {(plantView === "missions"  && farmData.missions != null) && <SettingsPanel sendData={memoizedSetDatatoSend} machineData={farmData} deleteMission={deleteMission} setDesiredPos={setDesiredPos} robotCmd={memoizedSetRobotCmd}/>}

            </div>
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
                <tbody>
                  <tr>
                    <td>Current:</td>
                    <td>{robotPos[0]}</td>
                    <td>{robotPos[1]}</td>
                    <td>{robotPos[2]}</td>
                  </tr>
                  <tr>
                    <td>Desired:</td>
                    <td>{desiredPos[0]}</td>
                    <td>{desiredPos[1]}</td>
                    <td>{desiredPos[2]}</td>
                  </tr>
                </tbody>
              </table>
          }
        />
        
        <OptionsView
          position={{bottom: '50px', right: '50px'}}
          content={
            <MovementControlPanel desiredPos={desiredPos} setDesiredPos={setDesiredPos} setRobotCmd={setRobotCmd}/>
          }
        />
    </>
  )
};

createRoot(document.getElementById('root')).render(<App />)
reportWebVitals();
