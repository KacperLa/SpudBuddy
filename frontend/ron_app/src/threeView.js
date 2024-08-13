import React, { createContext, useContext, useEffect, useMemo, useRef, useState } from 'react';
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

  if (size[0] == 0 || size[1] == 0 ) return null;

  const onPointerClick = (event) => {
    console.log("Farm clicked");
  }

  return (
    <mesh
        ref={farmRef}
        position={[0+size[0]/2, 0, 0+size[1]/2]}
        rotation={[-Math.PI / 2, 0, 0]}
        receiveShadow
        onDoubleClick={e => onPointerClick(e)}
        // onPointerMove={e => onPointerMove(e, meshRef.current)}
      >
        <planeGeometry args={size} />
        <meshStandardMaterial color="#F0F010" transparent={true} opacity={0.2} />
      </mesh>
  );
};

const SpudBuddy = ({ position }) => {
  const spudRef = useRef();

  return (
    <mesh
      ref={spudRef}
      position={position}
    >
      <sphereGeometry args={[.25, 32, 32]} />
      <meshStandardMaterial color="red" />
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
    camera.rotation.x = -3.14/2;
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
  const movingSphereRef = useRef();
  const [spheres, setSpheres] = useState([]);
  const [pcs, setPCs] = useState([]);
  const [pcInterval, setPcInterval] = useState(null);
  const [pointCloud, setPointCloud] = useState(null);

  function onPointerClick(event) {
    if (event.button !== 0) return;
    
    const newSphereRef = React.createRef();
    const newSphere = (
      <Sphere
        key={points.length}
        sphereRef={newSphereRef}
        position={event.point}
        isDraggingRef={movingSphereRef}
      />
    );
  
    setPoints(prevSpheres => [...prevSpheres, event.point]);
    setSpheres(prevSpheres => [...prevSpheres, newSphere]);
  }
  
  function onPointerMove(event) {
    if (!movingSphereRef.current)
    {
      // controlsRef.current.enabled = true;
      return;
    }
    // controlsRef.current.enabled = false;
    movingSphereRef.current.position.copy(event.point);

    let index = spheres.findIndex(sphere => sphere.props.sphereRef.current === movingSphereRef.current);
    let newPoints = [...points]; 
    newPoints[index] = event.point;
    setPoints(newPoints);
  }

  return (
    <Canvas id="canvas" camera={{position: [0, 10, 0]}}>
      <color attach="background" args={['#202020']} />
      <CameraController controlsRef={controlsRef} />
      <ambientLight intensity={1} />
      <gridHelper
        args={[200, 200]}
        position={[0, 0, 0]}
        opacity={100}
        color="white"
        colorCenterLine="yellow"
      />

      {spheres}

      <Farm size={[10,3]} />
      <SpudBuddy position={[0, 0, 0]} />

      <Path points={points} />
      
    </Canvas>
  );
}

export default ThreeView;
