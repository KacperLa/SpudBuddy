import { JoyStick } from './joy.js';

function joystickComponent(constainerId) {  
    // add style to the passed container
    var container = document.getElementById(constainerId);
    container.style.position = "fixed";
    container.style.bottom = "100px";
    container.style.right = "50px";
    container.style.width = "300px";
    container.style.height = "300px";
    container.style.margin = "50px";

    var joyParam = { "title": "joystick", "autoReturnToCenter": true };
    var joy = new JoyStick(constainerId, joyParam);

    function getJoyData() {
        return {
            x: joy.GetX(),
            y: joy.GetY(),
            timestamp: Date.now()
        };
    }
    return getJoyData;
}

export default joystickComponent;