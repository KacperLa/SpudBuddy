import React, {useState, useEffect} from 'react';
import ReactDOM from 'react-dom/client';
import { createRoot } from 'react-dom/client'
import { Canvas, useThree  } from '@react-three/fiber'
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls";
import * as THREE from "three";
import reportWebVitals from './reportWebVitals';

// import css styles
import './index.css';

// import custom components
import Joy from './joy.js';
import WebRTCComponent from './connection.js';



const CameraController = () => {
  const { camera, gl } = useThree();
  useEffect(
    () => {
      const controls = new OrbitControls(camera, gl.domElement);
      controls.minDistance = 3;
      controls.maxDistance = 20;
      return () => {
        controls.dispose();
      };
    },
    [camera, gl]
    );
    return null;
  };
  
function App() {
  const [xPos, setXPos] = useState(0);
  const [yPos, setYPos] = useState(0);
  
  function handleMove(event) {
    setXPos(event.x);
    setYPos(event.y);
  }
  
  function handleStop(event) {
    setXPos(0);
    setYPos(0);
  }

  //useEffect(
  //  () => {
  //    console.log("X: ", xPos, "Y: ", yPos)
  //    
  //  }, 
  //  [xPos, yPos]
  //);

  return (
    <div id="fullscreen-container">
      {/* <Canvas>
      <color attach="background" args={['#202020']} />
        <CameraController />
        <ambientLight intensity={1} />
        <directionalLight color="red" position={[0, 0, 5]} />
        <mesh>
          <boxGeometry args={[1, 1, 1]} />
          <meshStandardMaterial color="orange" />
        </mesh>
        
        <mesh position={[0, 0, 0]}>
          <planeGeometry args={[100, 100]} />
          <shadowMaterial color={"black"} opacity={1} />
          <planeGeometry rotateX={(-Math.PI / 2)}/>
        </mesh>

        <gridHelper args={[200, 200]} position={[0,0,0]} opacity={1} >
      
        </gridHelper>
      </Canvas> */}
      <WebRTCComponent joyXY={[xPos, yPos]}  />
      <Joy handleMove={handleMove} handleStop={handleStop} />
    </div>
  )
};


createRoot(document.getElementById('root')).render(<App />)
reportWebVitals();
