function videoComponent(constainerId) {  
    // add style to the passed container
    var container = document.getElementById(constainerId);
    container.innerHTML = "<img src='video_feed' style='width: 100%; height: auto;'>";


    container.style.position = "fixed";
    container.style.bottom = "100px";
    container.style.left = "50px";
    container.style.width = "300px";
    container.style.height = "300px";
    container.style.margin = "50px";

}

export default videoComponent;