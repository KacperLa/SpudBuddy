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

    console.log("joystickComponent: ", constainerId);

    var joyParam = { "title": "joystick", "autoReturnToCenter": true };
    var joy = new JoyStick(constainerId, joyParam);

    var joySocket = io();
    joySocket.on('connect', function() {
        joySocket.emit('joystick event', {data: 'I\'m connected!'});
    });

    setInterval(function()
        {
            var js_data = {
                x: joy.GetX(),
                y: joy.GetY(),
                timestamp: Date.now()
            };
            joySocket.emit('js', JSON.stringify(js_data));
        }, 100
    );
}

export default joystickComponent;