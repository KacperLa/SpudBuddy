import React, { createContext, useContext, useEffect, useMemo, useRef, useState } from 'react';
import { Canvas, useThree } from '@react-three/fiber';
import URDFLoader from 'urdf-loader';
import { PointerURDFDragControls } from 'urdf-loader/src/URDFDragControls';
import * as THREE from 'three';
import { BufferAttribute } from "three";

import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import { CatmullRomLine, PerspectiveCamera } from '@react-three/drei'
import { PointMaterial } from '@react-three/drei';

const robotURDFFilePath = '/static/RON/urdf/RON.urdf';

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


function CameraController({ controlsRef }) {
  const { camera, gl } = useThree();

  useEffect(() => {
    const controls = new OrbitControls(camera, gl.domElement);
    controls.minDistance = 3;
    controls.maxDistance = 20;
    controlsRef.current = controls;
    return () => {
      controls.dispose();
    };
  }, [camera, gl]);
  return (
    null
  );
}

function RobotModel({ robotRef, controlsRef }) {
  const { scene, camera, gl } = useThree();

  useEffect(() => {
    if (robotRef.current) {
      scene.add(robotRef.current);
      return;
    }

    const loader = new URDFLoader();
    loader.load(robotURDFFilePath, robot => {
      robot.scale.set(10, 10, 10);
      robot.traverse(c => {
        c.castShadow = true;
      });

      robotRef.current = robot; // Assign the robot to the ref
      scene.add(robot); // Add the robot directly to the scene
      console.log('Robot loaded');
    });

    return () => scene.remove(robotRef.current); // Clean up
  }, [robotRef, scene]);

  useEffect(() => {
    const dragControls = new PointerURDFDragControls(scene, camera, gl.domElement);

    dragControls.onDragStart = function (event) {
      controlsRef.current.enabled = false;
    };

    dragControls.onDragEnd = function (event) {
      controlsRef.current.enabled = true;
    };

    dragControls.onHover = joint => {
      joint.traverse(c => {
        if (c.type === 'Mesh') {
          c.material.emissive.set(0x0000ff);
        }
      });
    };

    dragControls.onUnhover = joint => {
      joint.traverse(c => {
        if (c.type === 'Mesh') {
          c.material.emissive.set(0x000000);
        }
      });
    };

    dragControls.updateJoint = (joint, angle) => {
      joint.setJointValue(angle);
      console.log(`Updated joint ${joint.name} to angle ${angle}`);
    };

    return () => {
      dragControls.dispose();
    };
  }, [camera, gl.domElement, scene, controlsRef]);

  return null; // No need to return an object from here
}

function ThreeView(props) {
  const meshRef = useRef();
  const [points, setPoints] = useState([]);
  const robotRef = useRef();
  const controlsRef = useRef();
  const movingSphereRef = useRef();
  const [spheres, setSpheres] = useState([]);
  const [pcs, setPCs] = useState([]);


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
      controlsRef.current.enabled = true;
      return;
    }
    controlsRef.current.enabled = false;
    movingSphereRef.current.position.copy(event.point);

    let index = spheres.findIndex(sphere => sphere.props.sphereRef.current === movingSphereRef.current);
    let newPoints = [...points]; 
    newPoints[index] = event.point;
    setPoints(newPoints);
  }

  const PointCloud = ({ pointCloud }) => {
    let points = useMemo(() => {
      console.log("MEMO");
      return new BufferAttribute(new Float32Array(pointCloud), 3);
    }, [pointCloud]);
    
    return (
      <points>
        <bufferGeometry>
          <bufferAttribute
            attach={"attributes-position"}
            {...points}
          />
        </bufferGeometry>
        <pointsMaterial
          size={5}
          color={0xff00ff}
          sizeAttenuation={false}
        />
      </points>
    );
  };

  return (
    <Canvas id="canvas" camera={{position: [1, 1, 7]}}>
      <color attach="background" args={['#202020']} />
      <CameraController controlsRef={controlsRef} />
      <ambientLight intensity={1} />
      <directionalLight color="red" position={[0, 0, 5]} />
      <gridHelper args={[200, 200]} position={[0, 0, 0]} opacity={1} />

      <mesh
        ref={meshRef}
        position={[0, 0, 0]}
        rotation={[-Math.PI / 2, 0, 0]}
        receiveShadow
        onDoubleClick={e => onPointerClick(e)}
        onPointerMove={e => onPointerMove(e, meshRef.current)}
      >
        <planeGeometry args={[200, 200]} />
        <meshStandardMaterial color="#F0F0F0" transparent={true} opacity={0.2} />
      </mesh>

      <RobotModel robotRef={robotRef} controlsRef={controlsRef} />
      
      {spheres}

      <Path points={points} />
      
      <PointCloud pointCloud={props.pointCloud}/>

    </Canvas>
  );
}

export default ThreeView;
