import { Joystick } from 'react-joystick-component';
import React from 'react'
import './joy.css';
import background_svg from "./assets/joy.svg";

function Joy(props) {
    return (
    <div id='joy'>
        <Joystick stickSize={90}  size={200} sticky={false} baseImage={background_svg} stickColor="black" move={props.handleMove} stop={props.handleStop}></Joystick>
    </div>
  );
}

export default Joy;
