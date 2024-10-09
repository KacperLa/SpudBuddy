import React, { useContext, useEffect, useMemo, useRef, useState } from 'react';
import { Canvas, useThree } from '@react-three/fiber';

import * as THREE from 'three';
import { BufferAttribute } from "three";

import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import { CatmullRomLine, PerspectiveCamera } from '@react-three/drei'
import { PointMaterial } from '@react-three/drei';

const Sphere = ({ sphereRef, position, isDraggingRef }) => {

  function onPointerDown(event) {
    isDraggingRef.current = sphereRef.current;
    event.stopPropagation();
  }

  function onPointerUp(event) {
    isDraggingRef.current = null;
    event.stopPropagation();
  }

  return (
    <mesh
      ref={sphereRef}
      position={position}
      onPointerDown={onPointerDown}
      onPointerUp={onPointerUp}
    >
      <sphereGeometry args={[1, 32, 32]} />
      <meshStandardMaterial color="blue" />
    </mesh>
  );
};

const Path = ({ points }) => {
  const lineRef = useRef();

  if (points.length < 2) return null;

  return (
    <CatmullRomLine
      ref={lineRef}
      points={points}
      curveType={"centripetal"}
      lineWidth={0.1}
      dashed={true}
      segments={100}
      worldUnits = {true}
    />
  );
};

const Farm = ({ size }) => {
  const farmRef = useRef();

  if (size[0] === 0 || size[1] === 0 ) return null;

  const onPointerClick = (event) => {
    console.log("Farm clicked");
  }

  return (
    <mesh
        ref={farmRef}
        position={[0+size[0]/2, 0+size[1]/2, 0]}
        rotation={[0, 0, 0]}
        receiveShadow
        onDoubleClick={e => onPointerClick(e)}
      >
        <planeGeometry args={size} />
        <meshStandardMaterial color="#F0F010" transparent={true} opacity={0.2} />
      </mesh>
  );
};

const Plant = ({ key, position }) => {
  const plantRef = useRef();
  return (
    <mesh
      ref={plantRef}
      position={position}
    >
      <sphereGeometry args={[.25, 32, .25]} />
      <meshStandardMaterial color="yellow" />
    </mesh>
  );
};

const Plants = ({plantData}) => {
  const plantRef = useRef();
  return (
    <>
      {plantData != null ? Object.keys(plantData).map((plant, index) => {
        return (  
          <Plant position={[plantData[plant].location[0]/100, plantData[plant].location[1]/100, 0]} />
        );  
      }) : null}
    </>
  );
};

const SpudBuddy = ({ position, color, setDesiredPos, isDraggable=false }) => {
  const spudRef = useRef();
  const [spudPos, setSpudPos] = useState(new THREE.Vector3(0, 0, 0));
  const [moving, setMoving] = useState(false);  

  function moveSpud(newPos) {
    if(!isDraggable) return; 
    if(!moving) return;
    setSpudPos([newPos.x, newPos.y, 0]);
  }

  function onRelease() {
    console.log("Spud clicked");
    setMoving(false);
    setDesiredPos([Math.floor(spudPos[0] * 100), Math.floor(spudPos[1] * 100), 0]);
  }

  useEffect(() => {
    let new_spudPos = new THREE.Vector3(position[0]/100, position[1]/100, 0);
    spudRef.current.position.copy(new_spudPos);
  }, [position]);

  return (
    <mesh
      ref={spudRef}
      position={spudPos}
      onPointerDown={() => setMoving(true)}
      onPointerUp={() => onRelease()}
      onPointerMove={(event) => moveSpud(event.point)}
    >
      <sphereGeometry args={[.25, 32, .25]} />
      <meshStandardMaterial color={color} />
    </mesh>
  );
};

function CameraController({ controlsRef }) {
  const { camera, gl } = useThree();

  useEffect(() => {
    const controls = new OrbitControls(camera, gl.domElement);
    controls.minDistance = 3;
    controls.maxDistance = 20;

    controls.enableRotate = false;

    // set camera rototation
    camera.rotation.x = 0;
    camera.rotation.y = 0;
    camera.rotation.z = 0;

    controlsRef.current = controls;
    return () => {
      controls.dispose();
    };
  }, [camera, gl]);
  return (
    null
  );
}

function ThreeView(props) {
  const meshRef = useRef();
  const [points, setPoints] = useState([]);
  const robotRef = useRef();
  const controlsRef = useRef();

  return (
    <Canvas id="canvas" camera={{position: [0, 0, 10]}}>
      <color attach="background" args={['#202020']} />
      <CameraController controlsRef={controlsRef} />
      <ambientLight intensity={1} />
      <gridHelper
        args={[200, 200]}
        position={[0, 0, 0]}
        rotation={[-Math.PI / 2, 0, 0]}
        opacity={100}
        color="white"
        colorCenterLine="yellow"
      />

      <Plants plantData={props.plantData}/>

      <Farm size={props.farmSize} />
      <SpudBuddy position={props.robotPos} color="red"/>
      <SpudBuddy position={props.desiredPos} setDesiredPos={props.setDesiredPos} color="blue" isDraggable={true}/>

      <Path points={points} />
      
    </Canvas>
  );
}

export default ThreeView;
