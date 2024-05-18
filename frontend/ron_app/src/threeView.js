import React, { useEffect, useRef, useState } from 'react';
import { Canvas, useThree } from '@react-three/fiber';
import URDFLoader from 'urdf-loader';
import { PointerURDFDragControls } from 'urdf-loader/src/URDFDragControls';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import { Raycaster } from 'three';

const robotURDFFilePath = '/static/RON/urdf/RON.urdf';

// const RobotModel = ({ robotRef }) => {
//   const { scene, camera, gl } = useThree();

//   useEffect(() => {
//     const loader = new URDFLoader();
//     loader.load(robotURDFFilePath, robot => {
//       robot.scale.set(10, 10, 10);
//       robot.traverse(c => {
//         c.castShadow = true;
//       });
//       robotRef.current.add(robot);
//       scene.add(robotRef.current); // Ensure the robot is added to the scene
//       console.log('Robot loaded');
//     });

//     // return () => {
//     //   scene.remove(robotRef.current); // Clean up the robot from the scene
//     // };
//   }, [robotRef, scene]);

//   useEffect(() => {
//     const dragControls = new PointerURDFDragControls(scene, camera, gl.domElement);

//     dragControls.onHover = joint => {
//       joint.traverse(c => {
//         if (c.type === 'Mesh') {
//           c.material.emissive.set(0xff0000);
//         }
//       });
//     };
//     dragControls.onUnhover = joint => {
//       joint.traverse(c => {
//         if (c.type === 'Mesh') {
//           c.material.emissive.set(0x000000);
//         }
//       });
//     };
//     dragControls.updateJoint = (joint, angle) => {
//       joint.setJointValue(angle);
//       console.log(`Updated joint ${joint.name} to angle ${angle}`);
//     };

//     return () => {
//       dragControls.dispose();
//     };
//   }, [camera, gl.domElement, scene]);

//   return <primitive object={new THREE.Object3D()} ref={robotRef} />;
// };


const RobotModel = ({ robotRef }) => {
  const { scene, camera, gl } = useThree();

  useEffect(() => {
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
  }, [camera, gl.domElement, scene]);


  return null; // No need to return an object from here
};

const CameraController = () => {
  const { camera, gl } = useThree();
  useEffect(() => {
    const controls = new OrbitControls(camera, gl.domElement);
    controls.mouseButtons = {
      LEFT: null,
      MIDDLE: THREE.MOUSE.ROTATE,
      RIGHT: THREE.MOUSE.PAN,
    };
    controls.minDistance = 3;
    controls.maxDistance = 20;
    return () => {
      controls.dispose();
    };
  }, [camera, gl]);
  return null;
};

const Sphere = ({ position }) => {
  return (
    <mesh position={position}>
      <sphereGeometry args={[1, 32, 32]} />
      <meshStandardMaterial color="blue" />
    </mesh>
  );
};

function ThreeView(props) {
  const meshRef = useRef();
  const raycaster = new Raycaster();
  const [spheres, setSpheres] = useState([]);

  function onPointerClick(event, planeRef, robotRef) {
    if (event.button !== 0) return;

    const pointer = new THREE.Vector2(
      (event.clientX / window.innerWidth) * 2 - 1,
      -(event.clientY / window.innerHeight) * 2 + 1
    );

    raycaster.setFromCamera(pointer, event.camera);

    const intersects = raycaster.intersectObjects([planeRef, robotRef], true);

    if (intersects.length > 0) {
      if (intersects[0].object === planeRef) {
        const intersectPoint = intersects[0].point;
        setSpheres(prevSpheres => [...prevSpheres, intersectPoint]);
      }
    }
  }

  return (
    <Canvas>
      <color attach="background" args={['#202020']} />
      <CameraController />
      <ambientLight intensity={1} />
      <directionalLight color="red" position={[0, 0, 5]} />
      <gridHelper args={[200, 200]} position={[0, 0, 0]} opacity={1} />

      <mesh
        ref={meshRef}
        position={[0, 0, 0]}
        rotation={[-Math.PI / 2, 0, 0]}
        receiveShadow
        onPointerDown={e => onPointerClick(e, meshRef.current, props.robotRef.current)}
      >
        <planeGeometry args={[200, 200]} />
        <meshStandardMaterial color="#F0F0F0" transparent={true} opacity={0.2} />
      </mesh>

      <RobotModel robotRef={props.robotRef} />
      
      {spheres.map((position, index) => (
        <Sphere key={index} position={position} />
      ))}
    </Canvas>
  );
}

export default ThreeView;
