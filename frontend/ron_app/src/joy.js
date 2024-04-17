import { Joystick } from 'react-joystick-component';
import React from 'react'
import './joy.css';

function Joy(props) {
    return (
    <div id='joy'>
        <Joystick size={100 } sticky={false} baseColor="red" stickColor="blue" move={props.handleMove} stop={props.handleStop}></Joystick>
    </div>
  );
}

export default Joy;
