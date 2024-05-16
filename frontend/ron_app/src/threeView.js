import React, { useEffect, useRef, useState } from 'react';
import { Canvas, useThree } from '@react-three/fiber';
import URDFLoader from 'urdf-loader';
import { PointerURDFDragControls } from 'urdf-loader/src/URDFDragControls';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls';
import { Raycaster } from 'three';

const robotURDFFilePath = '/static/RON/urdf/RON.urdf';

const RobotModel = () => {
  const robotRef = useRef(null);
  const { scene, camera, gl } = useThree();

  const highlightMaterial = new THREE.MeshBasicMaterial({ color: 0xff0000 });
  const standardMaterial  = new THREE.MeshBasicMaterial({ color: 0xa0a0a0 });

  const isJoint = j => j.isURDFJoint && j.jointType !== 'fixed';

  const highlightLinkGeometry = (m, revert) => {
    if (robotRef.current) {
      const traverse = c => {
        if (c.type === 'Mesh') {
          if (revert) {
            c.material = standardMaterial;
          } else {
            c.material = highlightMaterial;
          }
        }

        if (c === m || !isJoint(c)) {
          for (let i = 0; i < c.children.length; i++) {
            const child = c.children[i];
            if (!child.isURDFCollider) {
              traverse(child);
            }
          }
        }
      };
      traverse(m);
    }
  };

  const setJointValue = (joint, angle) => {
    
    // console.log(robotRef.current)
    if (robotRef.current) {
      robotRef.current.traverse(c => {
        if (c.name === joint) {
          c.setJointValue(angle);
        }
      });
    }
  };

  useEffect(() => {
    const loader = new URDFLoader();
    loader.load(robotURDFFilePath, robot => {
      if (robotRef.current) {
        robot.scale.set(10, 10, 10);
        robot.traverse(c => {
          c.castShadow = true;
        });
        robotRef.current.add(robot);
        scene.add(robotRef.current); // Ensure the robot is added to the scene
        console.log('robotRef is not null');
      } else {
        console.log('robotRef is null');
      }
    });

    return () => {
      if (robotRef.current) {
        scene.remove(robotRef.current); // Clean up the robot from the scene
      }
    };
  }, [robotURDFFilePath]);

  const dragControls = new PointerURDFDragControls(scene, camera, gl.domElement);

  dragControls.onHover = joint => {
    highlightLinkGeometry(joint, false);
  };
  dragControls.onUnhover = joint => {
    highlightLinkGeometry(joint, true);
  };
  dragControls.updateJoint = (joint, angle) => {
    setJointValue(joint.name, angle);
    const joint_data = {
      joint_name: joint.name,
      value: angle,
      timestamp: Date.now()
    };
    console.log(joint_data);
  };
 
  return <primitive object={new THREE.Object3D()} ref={robotRef} />;
};

const CameraController = ({ manipulating_robot }) => {
  const { camera, gl } = useThree();
  useEffect(() => {
    const controls = new OrbitControls(camera, gl.domElement);
    controls.mouseButtons = {
      LEFT: null,
      MIDDLE: THREE.MOUSE.ROTATE,
      RIGHT: THREE.MOUSE.PAN
    }	
    controls.minDistance = 3;
    controls.maxDistance = 20;
    return () => {
      controls.dispose();
    };
  }, [camera, gl, manipulating_robot]);
  return null;
};


const MarkerController = ({}) => {
  const { scene, camera, gl } = useThree();
  const raycaster = new Raycaster();
  let goToPoint = null;

  function onPointerClick(event) {
    console.log("onPointerClick");
    // let pointer = new THREE.Vector2();
    console.log(scene.getObjectById("plane"))
    // // event.preventDefault();
    // if ( event.button !== 0 ) return; 
    
    // // Calculate mouse position in normalized device coordinates (-1 to +1) for both components.
    // pointer.x = (event.clientX / window.innerWidth) * 2 - 1;
    // pointer.y = - (event.clientY / window.innerHeight) * 2 + 1;
  
    // raycaster.setFromCamera(pointer, camera);
  
    // const intersects = raycaster.intersectObject(scene.getObjectByName("plane"));
  
    // // If there's an intersection, add a sphere there.
    // if (intersects.length > 0) {
    //   const intersectPoint = intersects[0].point;
    //   if (goToPoint == null) {
    //     console.log("intersectPoint");
    //     // goToPoint = addSphere(intersectPoint);
    //     // scene.add(goToPoint);
    //   } else {
    //     // moveSphere(intersectPoint);
    //   }
    // }
    // if (goToPoint != null) {
    //   console.log(goToPoint.position);
    //   // sendDesiredPositon(goToPoint.position);
    // }
  }


  useEffect(() => {

    gl.domElement.addEventListener('mousedown', onPointerClick);

    return () => {
      gl.domElement.removeEventListener('mousedown', onPointerClick); 
    };

  }, []);
  return null;
};

function ThreeView() {
  return (
    <Canvas>
      <color attach='background' args={['#202020']} />
      <CameraController />
      <MarkerController />
      <ambientLight intensity={1} />
      <directionalLight color='red' position={[0, 0, 5]} />
      <gridHelper args={[200, 200]} position={[0, 0, 0]} opacity={1} />
      <plane id="plane" attach='geometry' args={[200, 200]} />
      <RobotModel />
    </Canvas>
  );
}

export default ThreeView;
