function sendPowerOption(name){
               
    let result = document.querySelector('.power_option_result');
    
    let xhr = new XMLHttpRequest();
    let url = "/action/" + name;

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({"socket": "sensor", "request": name });
    //if (confirm("Are you sure you want to shutdown the probe?") == true) {
    xhr.send(data);
    //}   
}

function sendAction(socket, request){
               
    let result = document.querySelector('.system_result');
    
    let xhr = new XMLHttpRequest();
    let url = "/action/" + request;

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({"socket": socket, "request": request });
    xhr.send(data);
}

function sendDroneStartIndexAction(){
               
    let result = document.querySelector('.system_result');
    let start_index = document.querySelector('#MISSION_INDEX');
    if (start_index.value == '') {
        start_index.value = 0
    }
    let xhr = new XMLHttpRequest();
    let url = "/action/load_mission";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({"socket": "drone", "request": "load_mission", "data": {"mission_start_index": Number(start_index.value)} });
    xhr.send(data);
}

function sendGetEnvData(){
    let result = document.querySelector('.env_result');

    let ul_parent = document.querySelector('.env_list-parent');
    var oListItem =  document.getElementById('env_list-dev').remove();  
    let table = document.createElement('table');
    table.id = 'env_list-dev'
    ul_parent.appendChild(table);

    let top_row = document.createElement('tr');

    let Sensor = document.createElement('th');
    Sensor.innerHTML += "Sensor";
    top_row.appendChild(Sensor);

    let Device = document.createElement('th');
    Device.innerHTML += "Device";
    top_row.appendChild(Device);

    let Value = document.createElement('th');
    Value.innerHTML += "Value";
    top_row.appendChild(Value);

    let Status = document.createElement('th');
    Status.innerHTML += "Status";
    top_row.appendChild(Status);

    table.appendChild(top_row)
    
    let xhr = new XMLHttpRequest();
    let url = "/action/get_env_status";

    xhr.open("GET", url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            const obj = JSON.parse(this.responseText);
            console.log(obj.data)
            Object.entries(obj.data).forEach(function ([key, item]) {
                    console.log(item)
                    let c_row = document.createElement('tr');

                    let c_Sensor = document.createElement('td');
                    c_Sensor.innerHTML += item.sensor;
                    c_row.appendChild(c_Sensor);

                    let c_device = document.createElement('td');
                    c_device.innerHTML += item.device;
                    c_row.appendChild(c_device);

                    let c_Value = document.createElement('td');
                    c_Value.innerHTML += item.value;
                    c_row.appendChild(c_Value);

                    let c_Status = document.createElement('td');
                    if (item.status){
                        c_Status.innerHTML += "OK";
                    } else {
                        c_Status.innerHTML += "FAIL";
                    }
                    c_row.appendChild(c_Status);

                    c_row.id = item.sensor
                    table.appendChild(c_row);
            });

        }
    };

    xhr.send();
}

function sendGetAction(request){
               
    let result = document.querySelector('.take_sample_result');
    
    let xhr = new XMLHttpRequest();
    let url = "/action/" + request;

    xhr.open("GET", url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    xhr.send();
}

function sensorFanToggle(){
               
    let state = document.querySelector('#SENSOR_STATE');
    let result = document.querySelector('.system_result');

    let xhr = new XMLHttpRequest();
    let url = "/action/sensor_fan";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({"state": state.checked });
    xhr.send(data);
}

function coolingFanToggle(){          
    let state = document.querySelector('#COOLING_STATE');
    let result = document.querySelector('.system_result');

    let xhr = new XMLHttpRequest();
    let url = "/action/cooling_fan";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({"state": state.checked });
    xhr.send(data);
}

function sendEnvStatus(name){
               
    let result = document.querySelector('.env_result');
    
    let xhr = new XMLHttpRequest();
    let url = "/action/" + name;
    let service = "sensor"

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({"socket": service, "request": name });
    xhr.send(data);
}

function sendGetBattery(){
               
    let result = document.querySelector('.get_battery_result');
    
    let xhr = new XMLHttpRequest();
    let url = "/action/get_battery_status";
    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({"socket": "sensor", "request": "get_battery_status" });
    xhr.send(data);
}

function sendWIFIAction(name){   
    let result = document.querySelector('.wifi_result');
    
    let xhr = new XMLHttpRequest();
    let url = "/wifi_configuration/" + name;

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({ "request": name });
    xhr.send(data);
}

function getSavedConnectionList(){
    let result = document.querySelector('.wifi_result');

    let ul_parent = document.querySelector('.wifi_list-parent');
    var oListItem =  document.getElementById('wifi_list-dev').remove();  
    let table = document.createElement('table');
    table.id = 'wifi_list-dev'
    ul_parent.appendChild(table);

    let top_row = document.createElement('tr');

    let CONNECTION_NAME = document.createElement('th');
    CONNECTION_NAME.innerHTML += "Connection";
    top_row.appendChild(CONNECTION_NAME);

    let CONNECTION_TYPE = document.createElement('th');
    CONNECTION_TYPE.innerHTML += "Type";
    top_row.appendChild(CONNECTION_TYPE);

    let CONNECTION_AUTOCONNECT = document.createElement('th');
    CONNECTION_AUTOCONNECT.innerHTML += "Auto Connect";
    top_row.appendChild(CONNECTION_AUTOCONNECT);

    let CONNECTION_ACTIVE = document.createElement('th');
    CONNECTION_ACTIVE.innerHTML += "Active";
    top_row.appendChild(CONNECTION_ACTIVE);

    let CONNECTION_DEVICE = document.createElement('th');
    CONNECTION_DEVICE.innerHTML += "Device";
    top_row.appendChild(CONNECTION_DEVICE);

    let CONNECTION_STATE = document.createElement('th');
    CONNECTION_STATE.innerHTML += "State";
    top_row.appendChild(CONNECTION_STATE);

    let CONNECTION_AUTOCONNECT_P = document.createElement('th');
    CONNECTION_AUTOCONNECT_P.innerHTML += "Priority";
    top_row.appendChild(CONNECTION_AUTOCONNECT_P);


    table.appendChild(top_row);
    
    let xhr = new XMLHttpRequest();
    let url = "/wifi_configuration/get_saved_connection_list";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            const obj = JSON.parse(this.responseText);
            console.log(obj)
            obj.data.forEach(function (item) {
                if (item.ssid != ""){
                    console.log(item)
                    let c_row = document.createElement('tr');
                    let remove_button = document.createElement('button');
                    let up_button = document.createElement('button');
                    let down_button = document.createElement('button');

                    let c_CONNECTION_NAME = document.createElement('td');
                    c_CONNECTION_NAME.innerHTML += item.name;
                    c_row.appendChild(c_CONNECTION_NAME);

                    let c_CONNECTION_TYPE = document.createElement('td');
                    c_CONNECTION_TYPE.innerHTML += item.type;
                    c_row.appendChild(c_CONNECTION_TYPE);

                    let c_CONNECTION_AUTOCONNECT = document.createElement('td');
                    c_CONNECTION_AUTOCONNECT.innerHTML += item.autoconnect;
                    c_row.appendChild(c_CONNECTION_AUTOCONNECT);

                    let c_CONNECTION_ACTIVE = document.createElement('td');
                    c_CONNECTION_ACTIVE.innerHTML += item.active;
                    c_row.appendChild(c_CONNECTION_ACTIVE);

                    let c_CONNECTION_DEVICE = document.createElement('td');
                    c_CONNECTION_DEVICE.innerHTML += item.device;
                    c_row.appendChild(c_CONNECTION_DEVICE);

                    let c_CONNECTION_STATE = document.createElement('td');
                    c_CONNECTION_STATE.innerHTML += item.state;
                    c_row.appendChild(c_CONNECTION_STATE);

                    let c_CONNECTION_AUTOCONNECT_P = document.createElement('td');
                    c_CONNECTION_AUTOCONNECT_P.innerHTML += item.autoconnectpriority;
                    c_row.appendChild(c_CONNECTION_AUTOCONNECT_P);

                    let c_remove = document.createElement('td');
                    c_remove.appendChild(remove_button);
                    c_row.appendChild(c_remove);
                    remove_button.onclick = function(){remove_connection(item.name)};
                    remove_button.innerHTML += 'REMOVE';
                    
                    let c_enable = document.createElement('td');
                    c_enable.appendChild(up_button);
                    c_row.appendChild(c_enable);
                    up_button.onclick = function(){up_connection(item.name)};
                    up_button.innerHTML += 'Enable';

                    let c_disable = document.createElement('td');
                    c_disable.appendChild(down_button);
                    c_row.appendChild(c_disable);
                    down_button.onclick = function(){down_connection(item.name)};
                    down_button.innerHTML += 'Disable';

                    c_row.id = item.ssid
                    table.appendChild(c_row);
                }
                
            });

        }
    };

    var data = JSON.stringify({ "request": "get_saved_connection_list"});
    xhr.send(data);
}

function remove_connection(name) {
    let result = document.querySelector('.wifi_result');

    let xhr = new XMLHttpRequest();
    let url = "/wifi_configuration/delete";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({ "request": "delete", "name": name});
    xhr.send(data);

}

function down_connection(name) {
    let result = document.querySelector('.wifi_result');

    let xhr = new XMLHttpRequest();
    let url = "/wifi_configuration/down";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({ "request": "down", "name": name});
    xhr.send(data);

}

function up_connection(name) {
    let result = document.querySelector('.wifi_result');

    let xhr = new XMLHttpRequest();
    let url = "/wifi_configuration/up";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({ "request": "up", "name": name});
    xhr.send(data);

}

function getConnectionList(){       
    let result = document.querySelector('.wifi_result');

    let ul_parent = document.querySelector('.wifi_list-parent');
    var oListItem =  document.getElementById('wifi_list-dev').remove();  
    let table = document.createElement('table');
    table.id = 'wifi_list-dev'
    ul_parent.appendChild(table);

    let top_row = document.createElement('tr');

    let CONNECTION = document.createElement('th');
    CONNECTION.innerHTML += "Connection";
    top_row.appendChild(CONNECTION);

    let DEVIVE = document.createElement('th');
    DEVIVE.innerHTML += "Device";
    top_row.appendChild(DEVIVE);

    let STATE = document.createElement('th');
    STATE.innerHTML += "State";
    top_row.appendChild(STATE);

    let TYPE = document.createElement('th');
    TYPE.innerHTML += "Type";
    top_row.appendChild(TYPE);

    let IP = document.createElement('th');
    IP.innerHTML += "IP address";
    top_row.appendChild(IP);

    table.appendChild(top_row);
    
    let xhr = new XMLHttpRequest();
    let url = "/wifi_configuration/connection_list";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            const obj = JSON.parse(this.responseText);
            obj.data.forEach(function (item) {
                if (item.ssid != ""){
                    console.log(item)
                    let c_row = document.createElement('tr');
                    
                    //let c_ACTIVE = document.createElement('th');
                    //c_ACTIVE.innerHTML += item.active;
                    //c_row.appendChild(c_ACTIVE);
                
                    let c_CONNECTION = document.createElement('td');
                    c_CONNECTION.innerHTML += item.connection;
                    c_row.appendChild(c_CONNECTION);

                    let c_DEVICE = document.createElement('td');
                    c_DEVICE.innerHTML += item.device;
                    c_row.appendChild(c_DEVICE);

                    let c_STATE = document.createElement('td');
                    c_STATE.innerHTML += item.state;
                    c_row.appendChild(c_STATE);

                    let c_TYPE = document.createElement('td');
                    c_TYPE.innerHTML += item.type;
                    c_row.appendChild(c_TYPE);

                    let c_IP = document.createElement('td');
                    c_IP.innerHTML += item.ipAddress;
                    c_row.appendChild(c_IP);

                    c_row.id = item.ssid
                    table.appendChild(c_row);
                }
                
            });

        }
    };

    var data = JSON.stringify({ "request": "connection_list"});
    xhr.send(data);
}

function sendWIFIList(){
               
    let result = document.querySelector('.wifi_result');
    let num_list = document.querySelector('#NUM_LIST');
    if (num_list.value == '') {
        num_list.value = 10
    }


    let ul_parent = document.querySelector('.wifi_list-parent');
    var oListItem =  document.getElementById('wifi_list-dev').remove();  
    let table = document.createElement('table');
    table.id = 'wifi_list-dev'
    ul_parent.appendChild(table);

    let top_row = document.createElement('tr');

    //let ACTIVE = document.createElement('th');
    //ACTIVE.innerHTML += "ACTIVE";
    //top_row.appendChild(ACTIVE);

    let SSID = document.createElement('th');
    SSID.innerHTML += "SSID";
    top_row.appendChild(SSID);

    let SECURITY = document.createElement('th');
    SECURITY.innerHTML += "SECURITY";
    top_row.appendChild(SECURITY);

    let SIGNAL = document.createElement('th');
    SIGNAL.innerHTML += "SIGNAL";
    top_row.appendChild(SIGNAL);

    table.appendChild(top_row)



    
    
    
    let xhr = new XMLHttpRequest();
    let url = "/wifi_configuration/network_list";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            const obj = JSON.parse(this.responseText);
            obj.data.forEach(function (item) {
                if (item.ssid != ""){
                    console.log(item)
                    let c_row = document.createElement('tr');
                    
                    //let c_ACTIVE = document.createElement('th');
                    //c_ACTIVE.innerHTML += item.active;
                    //c_row.appendChild(c_ACTIVE);
                
                    let c_SSID = document.createElement('td');
                    c_SSID.innerHTML += item.ssid;
                    c_row.appendChild(c_SSID);
                    let c_SECURITY = document.createElement('td');
                    c_SECURITY.innerHTML += item.security;
                    c_row.appendChild(c_SECURITY);
                    let c_SIGNAL = document.createElement('td');
                    c_SIGNAL.innerHTML += item.signal;
                    c_row.appendChild(c_SIGNAL);
                    c_row.id = item.ssid
                    table.appendChild(c_row);
                }
                
            });

        }
    };

    var data = JSON.stringify({ "request": "network_list", "first_nth_items": num_list.value });
    xhr.send(data);
}

function sendWIFIConnect(){
    let result = document.querySelector('.wifi_result');
    let ssid = document.querySelector('#SSID');
    let pass = document.querySelector('#PASSWORD');
    
    let xhr = new XMLHttpRequest();
    let url = "/wifi_configuration/connect";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({ "request": "connect", "ssid": ssid.value, "password": pass.value });
    xhr.send(data);
}

function createStaticETH(){
    let result = document.querySelector('.wifi_result');
    let name = document.querySelector('#STATICNAME');
    let ip = document.querySelector('#STATICADDRESS');
    
    let xhr = new XMLHttpRequest();
    let url = "/wifi_configuration/create_static_eth";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({ "request": "create_static_eth", "name": name.value, "ip": ip.value });
    xhr.send(data);
}

function sendTakeSample(){
               
    let result = document.getElementById('take_sample_result');
    let num_list = document.querySelector('#WO');
    
    let xhr = new XMLHttpRequest();
    let url = "/sample/capture";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({"work_order_id": WO.value });
    xhr.send(data);
}

function sendCreateMission(){        
    let result = document.querySelector('.mission_result');
    let wo_id = document.querySelector('#WO_ID'); 
    let user = document.querySelector('#USER');
    let job = document.querySelector('#JOB');
    let company = document.querySelector('#COMPANY');
    let g_stage = document.querySelector('#G_STAGE'); 
    let grower = document.querySelector('#GROWER'); 
    let field_name = document.querySelector('#FIELD_NAME');
    let crop_type = document.querySelector('#CROP_TYPE'); 
    let drone_mode = document.querySelector('#DRONE_MODE');
    let scout_mode = document.querySelector('#SCOUT_MODE'); 
    let apn_target = document.querySelector('#APN_TARGET');

    let xhr = new XMLHttpRequest();
    let url = "/mission/details";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };

    var data = JSON.stringify({ 'apn_target': apn_target.options[apn_target.selectedIndex].value, 
                                'drone_mode': drone_mode.checked, 
                                'scout_mode' : scout_mode.checked, 
                                'crop_type': crop_type.value,
                                'field_name': field_name.value,
                                'grower': grower.value,
                                'growth_stage': g_stage.value,
                                'company': company.value,
                                'job': job.value,
                                'user': user.value,
                                'work_order_id': wo_id.value
                            });
    xhr.send(data);
}

function getTmpImgs(){
    let ul_parent = document.querySelector('.mission_list-parent');
    var oListItem =  document.getElementById('mission_list-dev').remove();  
    let ul = document.createElement('mission_list-dev');
    ul.id = 'mission_list-dev'
    ul_parent.appendChild(ul);


    let xhr = new XMLHttpRequest();
    let url = "/sample/tmp_image_list";

    xhr.open("GET", url, true);
    
    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            const obj = JSON.parse(this.responseText);
            obj.data.forEach(function (item) {
                let li = document.createElement('li');
                let remove_button = document.createElement('button');
                let view_image_button = document.createElement('button');
                let download_image_button = document.createElement('button')
                let metadata_button = document.createElement('button');
                ul.appendChild(li);
                li.innerHTML += item;
                li.id = item
                li.appendChild(view_image_button);
                li.appendChild(download_image_button)
                li.appendChild(metadata_button);
                li.appendChild(remove_button);
                view_image_button.onclick = function(){location.href = ("/view_image/" + item)};
                download_image_button.onclick = function(){location.href = ("/get_file/image?file=" + item)};
                remove_button.onclick = function(){removeImage(item)};
                view_image_button.innerHTML += 'View';
                download_image_button.innerHTML += "Download"
                metadata_button.innerHTML += 'View Metadata';
                remove_button.innerHTML += 'REMOVE';
            });

        }
    };

    xhr.send();
}

function getMissionLogs() {
    let ul_parent = document.querySelector('.mission_list-parent');
    var oListItem =  document.getElementById('mission_list-dev').remove();  
    let ul = document.createElement('mission_list-dev');
    ul.id = 'mission_list-dev'
    ul_parent.appendChild(ul);


    let xhr = new XMLHttpRequest();
    let url = "/logs/mission_list";

    xhr.open("GET", url, true);
    
    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            const obj = JSON.parse(this.responseText);
            obj.data.forEach(function (item) {
                let li = document.createElement('li');
                let remove_button = document.createElement('button');
                let download_button = document.createElement('button')
                ul.appendChild(li);
                li.innerHTML += item;
                li.id = item
                li.appendChild(download_button)
                li.appendChild(remove_button);
                download_button.onclick = function(){location.href = ("/get_file/log?file=" + item)};
                remove_button.onclick = function(){removeLog(item)};
                download_button.innerHTML += "Download"
                remove_button.innerHTML += 'REMOVE';
            });

        }
    };

    xhr.send();
}

function getMissionList() {        
    let ul_parent = document.querySelector('.mission_list-parent');
    var oListItem =  document.getElementById('mission_list-dev').remove();  
    let ul = document.createElement('mission_list-dev');
    ul.id = 'mission_list-dev'
    ul_parent.appendChild(ul);


    let xhr = new XMLHttpRequest();
    let url = "/mission/list";

    xhr.open("GET", url, true);
    
    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            const obj = JSON.parse(this.responseText);
            obj.data.forEach(function (item) {
                let li = document.createElement('li');
                let remove_button = document.createElement('button');
                let edit_button = document.createElement('button');
                let view_batch_list = document.createElement('button');
                let get_file_list = document.createElement('button');
                let view_all_images = document.createElement('button');
                ul.appendChild(li);
                li.innerHTML += item;
                li.id = item
                li.appendChild(edit_button);
                li.appendChild(remove_button);
                li.appendChild(view_batch_list);
                li.appendChild(get_file_list);
                li.appendChild(view_all_images);
                remove_button.onclick = function(){removeMission(item)};
                edit_button.onclick = function(){editMission(item)};
                view_batch_list.onclick = function(){get_batch_list(item)};
                get_file_list.onclick = function(){location.href=("/file_list/"+item)}
                view_all_images.onclick = function(){ location.href=("/image_gallery/"+item)}
                remove_button.innerHTML += 'REMOVE';
                edit_button.innerHTML += 'Edit';
                view_batch_list.innerHTML += 'View Batch List';
                get_file_list.innerHTML += 'View File List'
                view_all_images.innerHTML += "View All Images"
            });

        }
    };

    xhr.send();
}

function removeMission(wo) {
    let result = document.querySelector('.mission_result');

    let xhr = new XMLHttpRequest();
    let url = "/mission/remove?wo=" + wo;

    xhr.open("GET", url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
            var oListItem =  document.getElementById(wo).remove();  
        }
    };
    xhr.send();
}

function removeImage(name) {
    let result = document.querySelector('.mission_result');

    let xhr = new XMLHttpRequest();
    let url = "/remove_file/image?file=" + name;

    xhr.open("GET", url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
            var oListItem =  document.getElementById(name).remove();  
        }
    };
    xhr.send();

}

function removeLog(name) {
    let result = document.querySelector('.mission_result');

    let xhr = new XMLHttpRequest();
    let url = "/remove_file/log?file=" + name;

    xhr.open("GET", url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
            var oListItem =  document.getElementById(name).remove();  
        }
    };
    xhr.send();
}

function remove_batch(id) {
    let result = document.querySelector('.mission_result');

    let xhr = new XMLHttpRequest();
    let url = "/sample/remove?id=" + id;

    xhr.open("GET", url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
            var oListItem =  document.getElementById(id).remove();  
        }
    };
    xhr.send();
}

function editMission(wo) {        

    let wo_id = document.querySelector('#WO_ID'); 
    let user = document.querySelector('#USER');
    let job = document.querySelector('#JOB');
    let company = document.querySelector('#COMPANY');
    let g_stage = document.querySelector('#G_STAGE'); 
    let grower = document.querySelector('#GROWER'); 
    let field_name = document.querySelector('#FIELD_NAME');
    let crop_type = document.querySelector('#CROP_TYPE'); 
    let drone_mode = document.querySelector('#DRONE_MODE');
    let scout_mode = document.querySelector('#SCOUT_MODE'); 
    let apn_target = document.querySelector('#APN_TARGET');

    let xhr = new XMLHttpRequest();
    let url = "/mission/details?wo=" + wo;

    xhr.open("GET", url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            const obj = JSON.parse(this.responseText);
            console.log(obj.data)
            wo_id.value = obj.data.work_order_id
            user.value = obj.data.user
            job.value = obj.data.job
            company.value = obj.data.company
            g_stage.value = obj.data.growth_stage
            grower.value = obj.data.grower
            field_name.value = obj.data.field_name
            crop_type.value = obj.data.crop_type
            drone_mode.checked = obj.data.drone_mode
            scout_mode.checked = obj.data.scout_mode
            if (obj.data.apn_target == "development"){
                apn_target.selectedIndex = 1
            } else {
                apn_target.selectedIndex = 0
            }
        };
    }
    xhr.send();
}

function get_batch_list(wo){
    let ul_parent = document.querySelector('.mission_list-parent');
    var oListItem =  document.getElementById('mission_list-dev').remove();  
    let ul = document.createElement('mission_list-dev');
    ul.id = 'mission_list-dev'
    ul_parent.appendChild(ul);


    let xhr = new XMLHttpRequest();
    let url = "/mission/batch_id?wo=" + wo;

    xhr.open("GET", url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            const obj = JSON.parse(this.responseText);
            obj.data.forEach(function (item) {
                let li = document.createElement('li');
                let remove_button = document.createElement('button');
                let view_batch_details = document.createElement('button');
                let list_images = document.createElement('button');
                let view_images = document.createElement('button');

                ul.appendChild(li);
                li.innerHTML += item;
                li.id = item;
                li.appendChild(remove_button);
                li.appendChild(view_batch_details);
                li.appendChild(list_images);
                li.appendChild(view_images);
                remove_button.onclick = function(){remove_batch(item)};
                view_batch_details.onclick = function(){get_batch_json(item)};
                list_images.onclick = function(){get_batch_images(item)};
                view_images.onclick = function(){location.href = ("/view_images/" + item)};
                remove_button.innerHTML += 'REMOVE';
                view_batch_details.innerHTML += 'View JSON';
                list_images.innerHTML += "List Images";
                view_images.innerHTML += "view Images";
            });

        }
    };
    xhr.send();
}

function get_batch_json(id){
    let url = "/sample/details?id=" + id;

    location.href = url;
}

function get_batch_images(id){
    let ul_parent = document.querySelector('.mission_list-parent');
    var oListItem =  document.getElementById('mission_list-dev').remove();  
    let ul = document.createElement('mission_list-dev');
    ul.id = 'mission_list-dev'
    ul_parent.appendChild(ul);


    let xhr = new XMLHttpRequest();
    let url = "/sample/image_list?id=" + id;

    xhr.open("GET", url, true);

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            const obj = JSON.parse(this.responseText);
            obj.data.forEach(function (item) {
                let li = document.createElement('li');
                let view_image = document.createElement('button');
                ul.appendChild(li);
                li.innerHTML += item;
                li.id = item;
                li.appendChild(view_image);
                view_image.onclick = function(){location.href = ("/view_image/" + item)};
                view_image.innerHTML += "View Image";
            });

        }
    };
    xhr.send();
}

function startDroneMission(){
    let result = document.querySelector('.take_sample_result');

    let drone_safety = document.querySelector('#DRONE_SAFETY');
    if (drone_safety.checked){
        let wo = document.querySelector('#WO');
        let xhr = new XMLHttpRequest();
        let url = "/action/start_drone_mission";
        xhr.open("POST", url, true);
        xhr.setRequestHeader("Content-Type", "application/json");

        xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
            }
        };

        var data = JSON.stringify({"socket": "drone", "request": "start_mission", "work_order_id": wo.value });
        xhr.send(data);
    } else {
        result.innerHTML = "Drone Safety NOT Checked."
    }
    
}

function sendStartStream(sensor_id){
    let result = document.querySelector('.result');       
    let xhr = new XMLHttpRequest();
    let url = "/action/start_stream";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({"socket": "camera", "request": "start_stream", "sensor_id": sensor_id });
    xhr.send(data);
}

function sendStopStream(){
                
    let result = document.querySelector('.result');
    
    let xhr = new XMLHttpRequest();
    let url = "/action/stop_stream";

    xhr.open("POST", url, true);

    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            result.innerHTML = this.responseText;
        }
    };
    var data = JSON.stringify({"socket": "camera", "request": "stop_stream" });
    xhr.send(data);
}
