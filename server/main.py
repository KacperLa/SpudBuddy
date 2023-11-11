#!venv/bin/python

import copy
from email import message
import click
from db_functions import *
from git_commands import *
from flask import Flask, send_file, request,render_template, current_app, g, abort
from flask.cli import with_appcontext
from generate_json import *
import glob
import json
import numpy as np
from multiprocessing import Value
import os
from os.path import exists
import random
from subprocess import check_output, Popen, PIPE, STDOUT
import time
from mmap_functions import *
from flask import Response
import io
from flask_socketio import SocketIO
import cv2
import mmap
import struct
import posix_ipc
import threading

DEVNULL = open(os.devnull, 'w')

sample_capture_busy = Value('b', 0)
sample_capture_time = Value('d', int(time.time()))

config_file_path = os.environ.get("CONFIG_JSON_FILE")
print(config_file_path)
if not config_file_path:
    raise ValueError("No config file provided.")
 
def check_if_running(process_name):
    pid = os.popen("pgrep "+process_name).read()
    return bool(pid)

def import_config(configFile):
    with open(configFile, "r") as config:
        params = json.load(config)
    return params

def shutdown(message=None):
    app_log.info("Running Shutdown script.")
    play_tone(config['tone_paths']['shutdown'])
    output = Popen([config['system_paths']['Shutdown']], stdin=PIPE, stdout=DEVNULL, stderr=STDOUT)  
    app_log.info(output)
    return {"success": True, "message": "Shutting Down"}

config = import_config(config_file_path)

MAP_NAME_ACTUAL  = '/tmp/robot_actual'
MAP_NAME_DESIRED = '/tmp/robot_desired' 
MAP_NAME_SETTINGS = '/tmp/robot_settings'

SEMAPHORE_NAME_ACTUAL  = "/robot_actual_sem"
SEMAPHORE_NAME_DESIRED = "/robot_desired_sem"
SEMAPHORE_NAME_SETTINGS = "/robot_settings_sem"


app = Flask(__name__)
socketio = SocketIO(app)

class ThreadSafeStruct:
    def __init__(self, data):
        self.data = data
        self.lock = threading.Lock()

    def set(self, key, value):
        with self.lock:
            self.data[key] = value

    def get(self, key):
        with self.lock:
            return self.data.get(key)

    def get_all(self):
        with self.lock:
            return self.data

    def remove(self, key):
        with self.lock:
            if key in self.data:
                del self.data[key]

# settings struct 
settings_format_mapping = {
    'pP': 'f',
    'pI': 'f',
    'pD': 'f',
    'vP': 'f',
    'vI': 'f',
    'vD': 'f',
    'yP': 'f',
    'yI': 'f',
    'yD': 'f',
    'pitchZero': 'f',
    'oP': 'f',
    'oI': 'f',
    'oD': 'f',
    'dP': 'f',
    'dI': 'f',
    'dD': 'f',
    'deadone': 'f'
}

settings_struct = {
    'pP': None,
    'pI': None,
    'pD': None,
    'vP': None,
    'vI': None,
    'vD': None,
    'yP': None,
    'yI': None,
    'yD': None,
    'pitchZero': None,
    'oP': None,
    'oI': None,
    'oD': None,
    'dP': None,
    'dI': None,
    'dD': None,
    'deadZone': None
}

settings = ThreadSafeStruct(settings_struct)
settings_mmap = mmap_writer(SEMAPHORE_NAME_SETTINGS, MAP_NAME_SETTINGS, settings_format_mapping)

# desired state struct
desired_state_format_mapping = {
    'state': 'i',
    'js_x': 'f',
    'js_y': 'f',
    'time': 'i',
    'pos_x': 'f',
    'pos_y': 'f'
}

desired_state_struct = {
    'state': 0,
    'js_x': 0.0,
    'js_y': 0.0,
    'time': 0,
    'pos_x': 0.0,
    'pos_y': 0.0
}

desired_state = ThreadSafeStruct(desired_state_struct) 
desired_state_mmap = mmap_writer(SEMAPHORE_NAME_DESIRED, MAP_NAME_DESIRED, desired_state_format_mapping)   


# actual state struct
actual_state_format_mapping = {
    'position_x': 'f',
    'position_y': 'f',
    'positionDeadReckoning_x': 'f',
    'positionDeadReckoning_y': 'f',
    'positionSlam_x': 'f',
    'positionSlam_y': 'f',
    'positionStatus': 'i',
    'orientation_roll': 'd',
    'orientation_pitch': 'd',
    'orientation_yaw': 'd',
    'angular_velocity_roll': 'd',
    'angular_velocity_pitch': 'd',
    'angular_velocity_yaw': 'd',
    'velocity': 'f',
    'left_velocity': 'f',
    'right_velocity': 'f',
    'state': 'i',
    'velocity_0': 'f',
    'position_0': 'f',
    'voltage_0': 'f',
    'state_0': 'i',
    'error_0': 'i',
    'velocity_1': 'f',
    'position_1': 'f',
    'voltage_1': 'f',
    'state_1': 'i',
    'error_1': 'i',
    'pP': 'f',
    'pI': 'f',
    'pD': 'f',
    'vP': 'f',
    'vI': 'f',
    'vD': 'f',
    'yP': 'f',
    'yI': 'f',
    'yD': 'f',
    'pitchZero': 'f'
}

actual_state_struct = {
    'position_x': 0.0,
    'position_y': 0.0,
    'positionDeadReckoning_x': 0.0,
    'positionDeadReckoning_y': 0.0,
    'positionSlam_x': 0.0,
    'positionSlam_y': 0.0,
    'positionStatus': 0,
    'orientation_roll': 0.0,
    'orientation_pitch': 0.0,
    'orientation_yaw': 0.0,
    'angular_velocity_roll': 0.0,
    'angular_velocity_pitch': 0.0,
    'angular_velocity_yaw': 0.0,
    'velocity': 0.0,
    'left_velocity': 0.0,
    'right_velocity': 0.0,
    'state': 0,
    'pP': 0.0,
    'pI': 0.0,
    'pD': 0.0,
    'vP': 0.0,
    'vI': 0.0,
    'vD': 0.0,
    'yP': 0.0,
    'yI': 0.0,
    'yD': 0.0,
    'pitchZero': 0.0
}

actual_state = ThreadSafeStruct(actual_state_struct)
actual_state_mmap = mmap_reader(SEMAPHORE_NAME_ACTUAL, MAP_NAME_ACTUAL, actual_state_format_mapping)

@app.route("/")
def home():
    return render_template('home.html')

def generate_data():
    data_json = config['robotState']
    while True:
        desired_state_mmap.write(desired_state.get_all())
        
        actual_data = actual_state_mmap.read(actual_state_struct)
        for key in actual_data.keys():
            data_json['actual'][key] = actual_data[key] 
        yield f"data: {json.dumps(data_json)}\n\n"
        time.sleep(.1)
    semaphore.close()

def generate_log_data():
    while True:
        pass
        # socks = dict(poller.poll(0))
        # if sock in socks and socks[sock] == zmq.POLLIN:
        #     message = sock.recv_string(zmq.NOBLOCK)
        #     data = json.loads(message)
        #     print(message)
        #     yield f"data: {json.dumps(data)}\n\n"
        # else:
        #     time.sleep(.05)
        #     continue

@app.route('/log_stream')
def log_data():
    return Response(generate_log_data(), mimetype='text/event-stream')

@app.route('/data')
def data():
    return Response(generate_data(), mimetype='text/event-stream')


@app.route('/tunning')
def tunning():
    return render_template('tunning.html')

@app.route('/3dview')
def threeDView():
    return render_template('3dview.html')

@app.route('/path')
def path():
    return render_template('path.html')

@app.route('/ron')
def ron():
    return render_template('ron.html')

@socketio.on('js')
def handle_message(message):
    # put x and y in desired state
    data = json.loads(message)
    # convert string to floats
    desired_state.set('js_x', float(data['x']))
    desired_state.set('js_y', float(data['y']))

@app.route('/request_position', methods=['POST'])
def request_position():
    # ensure that the position is a float
    desired_state.set('pos_x', float(request.json.get('pos_x')))
    desired_state.set('pos_y', float(request.json.get('pos_y')))
    print("got updated position", desired_state.get('pos_x'), desired_state.get('pos_y'))
    return {"success": True}

@app.route('/request_state')
def request_state():
    # get state from args and set it to desired state
    # ensure that the state is an int
    desired_state.set('state', int(request.args.get('state')))
    print(desired_state.get('state'))
    return {"success": True}

@app.route('/request_settings', methods=['GET', 'POST'])
def request_settings():
    # is the request is a post
    if request.method == "POST":
        # get settings from args and set it to desired
        req_settings = request.json.get('settings', None)
        if req_settings is not None:
            for item in req_settings.keys():
                if item in settings.data.keys():
                    settings.set(item, float(req_settings[item]))
            settings_mmap.write(settings.get_all())
    else:
        # return settings from actual state
        for key in settings_struct.keys():
            settings.set(key, actual_state.get(key))

    return {"success": True, "settings": settings.get_all()}


@app.route("/file_list/<path:subpath>", methods=['GET'])
def file_list(subpath):
    data = None
    success = False
    work_order = subpath
    if str(work_order).isnumeric():
        data = {}
        data['image_list'] = []
        data['classification_json_list'] = []
        data['log_list'] = []
        data['export_csv_list'] = []
        data['zip_list'] = []
        success, batch_list = get_batch_list_for_wo(int(work_order))
        for batch_id in batch_list:
            has_image, image_list = get_image_files_by_batch(int(batch_id), config['system_paths']['image_path'])
            data['image_list'].extend(image_list)
            classification_json = get_json_name_by_batch(str(batch_id), config['system_paths']['export_path'])
            if classification_json is not None:
                data['classification_json_list'].append(classification_json)
            else:
                success, camera_data = get_image_files_by_batch(int(batch_id), config['system_paths']['image_path'])
                image_data =  [({'name': img}) for img in camera_data if img is not None] 
                _ = generate_json_from_db(batch_id, PROBE_ID, export_path=config['system_paths']['export_path'], camera_data = image_data)
                classification_json = get_json_name_by_batch(str(batch_id), config['system_paths']['export_path'])
                data['classification_json_list'].append(classification_json)

            csv_list = get_csv_name_by_batch(str(batch_id), config['system_paths']['export_path'])
            for csv in csv_list:
                data['export_csv_list'].append(csv)

            success = True
    return {"success": success,
            "data": data}

@app.route("/remove_file/<path:subpath>", methods=['GET'])
def remove_image(subpath):
    file_name = request.args.get('file')
    if file_name is not None:
        if subpath == "image":
            file_path = os.path.join(config['system_paths']['image_path'], file_name)
        elif subpath == "log":
            file_path = os.path.join(config['system_paths']['log_path'], file_name)

        if exists(file_path):
            try:
                os.remove(file_path)
                success = True
                message = ("File %s removed successfully." %file_name)
            except OSError as error:
                success = False
                message = ("An error occured when removing %s." %file_name)
        else:
            success = False
            message = ("The specified file \"%s\" does not exist." %file_name) 
    
    return {"success": success,
            "data": message}

@app.route("/view_file/<path:subpath>")
def view_file(subpath):
    os_path = None
    file_name = request.args.get('file')
    if subpath == "image":
        if file_name is not None:
            os_path = config['system_paths']['image_path']
            file_path = os.path.join(os_path, file_name)
            if not exists(file_path):
                abort(404, description="Image not found")
    else:
        abort(404, description="path only for images.")

    return send_file(file_path , download_name=file_name, as_attachment=True)

@app.route("/get_waypoint_list/<path:subpath>")
def get_waypoint_list(subpath):
    wo_id = subpath
    waypoint_list = {}
    batch_success, batch_data = get_batch_list_for_wo(int(wo_id))
    for batch in batch_data:
        drone_data = get_drone_data_by_batch_id(batch)
        env_data = get_env_sensor_data_by_batch_id(batch)
        waypoint = drone_data["waypoint"]
        if not waypoint in waypoint_list.keys():
            waypoint_list[waypoint] = {}
        
        if not batch in waypoint_list[waypoint].keys():
            waypoint_list[waypoint][batch] = {}
            waypoint_list[waypoint][batch]["drone"] = {}
            waypoint_list[waypoint][batch]["env"] = {}
            #waypoint_list[waypoint][batch]["images"] = {}

        
        waypoint_list[waypoint][batch]["drone"] = drone_data
        waypoint_list[waypoint][batch]["env"] = drone_data
        #image_success, waypoint_list[waypoint][batch]["images"] = get_image_files_by_batch(batch, config['system_paths']['image_path'])
    return waypoint_list

@app.route("/get_file/<path:subpath>")
def get_file(subpath):
    os_path = None
    file_type = None
    file_name = request.args.get('file')
    if file_name is not None:
        if subpath == "export_csv":
            os_path = config['system_paths']['export_path']
        elif subpath == "image":
            os_path = config['system_paths']['image_path']
        elif subpath == "log":
            os_path = config['system_paths']['log_path']
        elif subpath == "zip":
            os_path = config['system_paths']['log_path']
        elif subpath == "classification_json":
            os_path = config['system_paths']['export_path']
    
    file_path = os.path.join(os_path, file_name)
    if not exists(file_path):
        abort(404, description="File not found")

    return send_file(file_path , download_name=file_name, as_attachment=True)

@app.route("/tag/list", methods=['GET'])
def tag_list():
    return get_git_tag_list()

@app.route("/tag/checkout")
def tag_checkout():
    req_tag = request.args.get('tag', None)
    if req_tag is None:
        return {"success": False, "data": "no tag provided."}

    tag_list = get_git_tag_list()
    if req_tag not in tag_list.get("data", []):
        return {"success": False, "data": "invalid tag."}
    else:
        return git_checkout_tag(config, req_tag)

@app.route("/view_chart/<subpath>")
def view_chart(subpath):
    message_list = config['actions']['message_list']
    log_paths = config['log']['path']['sensors']
    
    message = None
    data = {}
    labels = []
    values = []
    sensor_list = []
    device = None

    if subpath in message_list:
        message = copy.copy(message_list[subpath])
        if message['socket'] == "sensor":
            _, data = sensor_socket.query(message)
            _data_keys = list(data.keys())
            print(_data_keys)
            if "device" not in _data_keys:
                device = data.get(_data_keys[0], {}).get("device")
            else:
                device = data.get('device')

            path_to_log = log_paths[device]
            try:                
                if exists(path_to_log):
                    with open(path_to_log, "r") as log_file:
                        log = log_file.readlines()[-300:]
                    for line in log:
                        _line = line.rstrip().split(" ")
                        
                        labels.append(_line[0].split(",")[1].split(".")[0])
                        values.append(list(map(float, _line[1].split(","))))
                    print(labels)

                values = np.asarray(values).T
                if "device" not in _data_keys:
                    print("no dev", _data_keys)
                    for sensor, value_set in zip(_data_keys, values):
                        data[sensor]["values"] = list(value_set)
                        sensor_list.append(data.get(sensor))
                else:
                    print("dev")
                    data["values"] = list(values)[0]
                    sensor_list.append(data)
            except Exception as e:
                print(e)
    else:
        print("Invalid Socket.")
        print(sensor_list)

    return render_template('chart.html', title=device, labels=labels, sensor_list=sensor_list)

@app.route("/view_image/<image_name>")
def view_image(image_name):
    return render_template('view_image.html', image_file = image_name)

@app.route("/view_images/<path:subpath>")
def view_images(subpath):
    batch_id = subpath
    _, image_list = get_image_files_by_batch(int(batch_id), config['system_paths']['image_path'])
    print(image_list)
    return render_template('view_images.html', image_list = image_list)

@app.route("/image_gallery/<path:subpath>")
def view_image_gallery(subpath):

    wo_id = subpath
    waypoint_list = {}
    waypoint_list["waypoints"] = {}
    waypoint_list["image count"] = 0
    waypoint_list["batch count"] = 0
    waypoint_list["wo"] = wo_id

    batch_success, batch_data = get_batch_list_for_wo(int(wo_id))
    for batch in batch_data:
        drone_data = get_drone_data_by_batch_id(batch)
        env_data = get_env_sensor_data_by_batch_id(batch)
        waypoint = drone_data["waypoint"] if drone_data != {} else 9999
        if not waypoint in waypoint_list["waypoints"].keys():
            waypoint_list["waypoints"][waypoint] = {}
            waypoint_list["waypoints"][waypoint]["image count"] = 0
            waypoint_list["waypoints"][waypoint]['batch count'] = 0
            waypoint_list["waypoints"][waypoint]['batches'] = {}

        if not batch in waypoint_list["waypoints"] [waypoint]['batches'].keys():
            waypoint_list["waypoints"][waypoint]['batches'][batch] = {}
            waypoint_list["waypoints"][waypoint]['batches'][batch]["drone"] = {}
            waypoint_list["waypoints"][waypoint]['batches'][batch]["env"] = {}
            waypoint_list["waypoints"][waypoint]['batches'][batch]["images"] = {}
            waypoint_list["waypoints"][waypoint]['batches'][batch]["image count"] = 0

        waypoint_list["waypoints"][waypoint]['batches'][batch]["drone"] = drone_data
        waypoint_list["waypoints"][waypoint]['batches'][batch]["env"] = drone_data
        image_success, waypoint_list["waypoints"][waypoint]['batches'][batch]["images"] = get_image_files_by_batch(batch, config['system_paths']['image_path'])
        waypoint_list["waypoints"][waypoint]['batches'][batch]["image count"] = len(waypoint_list["waypoints"][waypoint]['batches'][batch]["images"])

    
    for waypoint in waypoint_list["waypoints"] .keys():
        for batch in waypoint_list["waypoints"][waypoint]['batches'].keys():
            waypoint_list["waypoints"][waypoint]["image count"] += len(waypoint_list["waypoints"][waypoint]['batches'][batch]['images'])
            waypoint_list["image count"] += len(waypoint_list["waypoints"][waypoint]['batches'][batch]['images'])
        waypoint_list["waypoints"][waypoint]['batch count'] = len(waypoint_list["waypoints"][waypoint]['batches'])
        waypoint_list['batch count'] += len(waypoint_list["waypoints"][waypoint]['batches'])

    return render_template('image_gallery.html', waypoint_list = waypoint_list)

@app.route("/view_stream")
def view_stream():
    return render_template('stream.html')

@app.route("/video_feed_0")
def video_feed_0():
	return Response(generate(0), mimetype = "multipart/x-mixed-replace; boundary=frame")

@app.route("/video_feed_1")
def video_feed_1():
	return Response(generate(), mimetype = "multipart/x-mixed-replace; boundary=frame")

def generate():
    while True:
        pass
        # socks = dict(poller.poll(0))
        # if sock in socks and socks[sock] == zmq.POLLIN:
        #     data = sock.recv(zmq.NOBLOCK)
        #     yield(b'--frame\r\n' b'Content-Type: image/jpeg\r\n\r\n' + data + b'\r\n')
        # else:
        #     time.sleep(.05)
        #     continue

@app.route("/map_stream")
def map_stream():
    return Response(generate_map(), mimetype='text/event-stream')

def generate_map():
    pass
    # sock = context.socket(zmq.SUB)
    # sock.connect(("tcp://localhost:5500"))
    # sock.subscribe(b"")
    # poller = zmq.Poller()
    # poller.register(sock, zmq.POLLIN)
    # while True:
    #     socks = dict(poller.poll(0))
    #     if sock in socks and socks[sock] == zmq.POLLIN:
    #         message = sock.recv(zmq.NOBLOCK)
    #         data = json.loads(message)
    #         yield f"data: {json.dumps(data)}\n\n"
    #     else:
    #         time.sleep(.05)
    #         continue

@app.route("/video_feed_2")
def video_feed_2():
	return Response(generate(2), mimetype = "multipart/x-mixed-replace; boundary=frame")

@app.route("/wifi_configuration/<path:subpath>", methods=['GET', 'POST'])
def wifi_configuration(subpath):
    success = None
    data = None
    content = []

    message_list = config['wifi_configuration']['message_list']
    message = None

    if request.method == "POST":
        content = request.json
        if content is None:
            success = False
            data = "Did not receive JSON object."
    
    if success is not False:
        if subpath in message_list:
            message = copy.copy(message_list[subpath])
            for item in content:
                if item in message:
                    message[item] = content[item]
            if None in message.values():
                success = False
                data = "Request is missing neccessary keys."
            print(message)
        else:
            success = False
            data = ("Request %s does not exist." %subpath)
        
        if success is None:
            success, data = wifi_socket.query(message)
        else:
            success = False
            data = "Request not proccessed."
        
    return {"success": success,
            "data": data}

@app.route("/load_mission_file", methods=['POST'])
def load_plan_file():
    success = None
    data = None
    file_name = request.files['file'].filename
    if file_name == '':
        data = 'No file part'
        success = False
    else:
        file = request.files['file']
        file_path = os.path.join(config["system_paths"]["flight_plans"], file_name)
        file.save(file_path)
        mission_json = None
        try:
            with open(file_path, "r") as json_file:
                mission_json = json.load(json_file)
        except:
            success, data = False, "File is not a valid JSON."
        
        if success != False:
            if 'aker-mission-details' in mission_json.keys():
                mission_json['aker-mission-details']['flight_plan'] = file_path
                mission_json['aker-mission-details']['work_order'] = str(mission_json['aker-mission-details']['work_order'])
                mission_json['aker-mission-details']['mission_file'] = str(mission_json['aker-mission-details']['work_order'])+".plan"
                message = {"request": "load_mission", "data": mission_json['aker-mission-details']}
                #print(json.dumps(mission_json, indent=4, sort_keys=True))

                if "developme0nt" in mission_json['aker-mission-details'].keys():
                    success = True
                    data = "File uploaded successfully."
                    print("HEOOO")
                else:
                    if check_if_running("AkerSC2"):
                        success, data = drone_socket.query(message)
                    else:
                        success, data = False, "This service is down."
            else:
                success, data = False, "Mission file is missing aker-mission-details."

    return {"success": success,
            "data": data}

@app.route("/mission/list", methods=['GET'])
def mission_list():
    success, data = get_workorder_list()
    return {"success": success,
            "data": data}

@app.route("/mission/remove", methods=['GET'])
def mission_remove():
    success = None
    data = None
    wo = int(request.args.get('wo'))
    success, wo_list = get_workorder_list()
    if success != True:
        data = "Falied to retrive workorder list, plese try again."
        print(data)
    else:
        if wo in wo_list:
            _ , data = get_batch_list_for_wo(wo)
            for id in data:
                _ , data = remove_batch_from_db(id)
                remove_image_files_by_batch(id, config['system_paths']['image_path'])
                remove_json_file_by_batch(id, config['system_paths']['export_path'])  
            success, data = remove_mission_from_db(wo)
        else:
            success = False
            data = "The specified workorder %d does not exist in the data db." %wo
            print(data)
    return {"success": success,
            "data": data}

@app.route("/mission/batch_id", methods=['GET'])
def mission_batch_id():
    wo = request.args.get('wo')
    if str(wo).isnumeric():
        success, data = get_batch_list_for_wo(int(wo))
    else:
        success, data = False, "Workorder is not numberic."
    return {"success": success,
            "data": data}

@app.route("/mission/details", methods=['GET', 'POST'])
def mission_details():
    success = None
    data = None
    if request.method == "GET":
        wo = request.args.get('wo')
        success, data = get_mission_details(wo)
    elif request.method == "POST":
        content = request.json
        mission_info = config['mission']
        if content is None:
            success = False
            data = "Did not receive JSON object."
        else:
            for item in content.keys():
                if item in mission_info['info'].keys():
                    mission_info['info'][item] = content[item]
            if None in mission_info['info'].values() or '' in mission_info['info'].values():
                success = False
                data = { "message": "Request is missing neccessary keys.", "data": copy.copy(mission_info)}
                for item in mission_info['info'].keys():
                    mission_info['info'][item] = None
            else:
                wo_list_success, work_order_list = get_workorder_list()
                if int(mission_info['info']['work_order_id']) in work_order_list:
                    success, data = update_mission_details(mission_info['info'])
                else:
                    success, data = add_mission_to_db(work_order_id=mission_info['info']["work_order_id"],
                                                        grower=mission_info['info']["grower"],
                                                        field_name=mission_info['info']["field_name"],
                                                        crop_type=mission_info['info']["crop_type"],
                                                        growth_stage=mission_info['info']["growth_stage"],
                                                        user=mission_info['info']["user"],
                                                        company=mission_info['info']["company"],
                                                        job=mission_info['info']["job"],
                                                        scout_mode=mission_info['info']["scout_mode"],
                                                        drone_mode=mission_info['info']["drone_mode"],
                                                        apn_target=mission_info['info']["apn_target"])
    return {"success": success,
            "data": data}

@app.route("/mission/downloaded_status/<path:subpath>", methods=['GET'])
def mission_downloaded_status(subpath):
    success = False
    data = None
    state = request.args.get('state', None)
    if str(subpath).isnumeric():
        wo_list_success, work_order_list = get_workorder_list()
        if int(subpath) in work_order_list:
            if state is None:
                success, data = get_mission_downloaded_status(int(subpath))
            else:
                if state.isnumeric():
                    success, data = update_mission_downloaded(int(subpath), int(state))
                else:
                    state = False
                    data = "state must be 0 if false and 1 or grater if true"
        else:
            status = False
            data = "Work order does not exist in the DB"                   
    else:
        success = False
        data = "work order number is not numeric"
    return {"success": success,
            "data": data}

@app.route("/sensors/<path:subpath>", methods=['GET'])
def sensors(subpath):
    success = False
    log_paths = config['log']['path']['sensors']

    if subpath in log_paths:
        path_to_log = log_paths[subpath]
        try:                
            if exists(path_to_log):
                with open(path_to_log, "r") as log_file:
                    data = log_file.readlines()[-1].rstrip()
                success = True
            else:
                success = False
                data = ("File for log %s does not exist." %subpath)
        except Exception as e:
            success = False
            data = ("Erroe getting %s log file: %s" %(subpath, e))
            print(e)
    else:
        success = False
        data = ("No log with name: '%s'." %subpath)
    return {"success": success,
            "data": data}

@app.route("/logs/mission_list")
def mission_log_list():
    success, data = get_mission_log_files(config['system_paths']['log_path'])
    return {"success": success,
            "data": data}

@app.route("/logs/<path:subpath>")
def logs(subpath):
    log_paths = config['log']['path']
    path_to_log = None
    for section in log_paths:
        if subpath in log_paths[section]:
            path_to_log = log_paths[section][subpath]
    
    return send_file(path_to_log , download_name=os.path.basename(path_to_log), as_attachment=True)

@app.route("/sample/capture", methods=['POST'])
def sample_capture():
    busy = False
    with sample_capture_busy.get_lock():
        if sample_capture_busy.value != 0:
            if (sample_capture_time.value - int(time.time())) < 15:
                busy = True
            else:
                sample_capture_busy.value = 0
        
        print(sample_capture_busy.value)

    if True:
        with sample_capture_busy.get_lock():
            sample_capture_busy.value = 1
            sample_capture_time.value = int(time.time())
        
        data = None
        success = None
        content = request.json

        mission_info = None
        PROBE_ID = 1111

        _ , batch_id = increment_batch_id(config['system_paths']['batch_id_path'])
        epoch = int(time.time())
        print(content)
        wo = str(content.get('work_order_id', None)) if len(str(content.get('work_order_id', ''))) > 0 else None
        if wo is not None:
            if verify_workorder_number(wo):
                basename = f"{config['system_paths']['image_path']}/{batch_id}-{epoch}-{PROBE_ID}-1-"
            else:
                success = False
                data = "Invalid work_order_id"
        else:
            basename = f"{config['system_paths']['image_path']}/{batch_id}-{epoch}-{PROBE_ID}-tmp-"

        if success is None:
            time_start = time.time()
            sensor_success, sensor_data = sensor_socket.query({"request": "get_env_status"})
            print("Sensor_time: ", time.time() - time_start, sensor_success)
            gps_success, gps_data = gps_socket.query({"request": "get_gps_status"})
            # print("GPS_time: ", time.time() - time_start, gps_success)

            if (check_if_running("AkerSC2")):
                drone_success, drone_data = drone_socket.query({"request": "capture_info"})
                threaded_save = True
                focus_type = "laplace"
            else:
                drone_success, drone_data = None, None
                threaded_save = False
                focus_type = "center"
                
            #camera_success, camera_data = camera_socket.query({"request": "capture_sample", "base_name": basename, "gps_data": gps_data, "sensor_data": sensor_data, "drone_data": drone_data, "threaded_save": threaded_save, "focus_type": focus_type}, timeout=3000)
            #print("Camera_time: ", time.time() - time_start, camera_success)
           # 
           # if ((True in camera_success) if isinstance(camera_success, list) else camera_success) and sensor_success:
           #     play_tone(config['tone_paths']['shutter'])
           # else:
           #     play_tone(config['tone_paths']['alarm'])

            #print(wo)
            #print(camera_data)
            #print(sensor_data)
            #print(gps_data)

            if wo is not None and  ((True in camera_success) if isinstance(camera_success, list) else camera_success) and sensor_success:
                print("Adding to the db.")
                success, mission_info = get_mission_details(wo)
                add_batch_success, add_batch_data = add_batch_to_db(work_order_id=wo, local_batch=batch_id, epoch=epoch, notes=content.get('notes'), stress_items=content.get('stress_items'))

                gps_db_success, gps_db_data = add_gps_to_db(gps_data, batch_id, epoch)        
                sensor_db_success, senosr_db_data = add_env_sensor_data_to_db(local_batch      = batch_id,
                                                                        temperature            = sensor_data["Temperature"]['value'],
                                                                        humidity               = sensor_data["Humidity"]['value'],
                                                                        pressure               = sensor_data["Pressure"]['value'],
                                                                        carbon_dioxide         = sensor_data["CO2"]['value'],
                                                                        oxygen                 = sensor_data["O2"]['value'],
                                                                        internal_upper_celsius = sensor_data["Internal_Temperature"]['value'],
                                                                        grav                   = sensor_data['IMU']['value'][9:],
                                                                        epoch                  = epoch)
                if (drone_success):
                    drone_db_success, drone_db_data = add_drone_data_to_db(drone_data, batch_id, epoch)
                
                success = True

                image_data =  [({'name': img.get('name')}) for img in camera_data if img is not None] 
                data = generate_json_from_db(batch_id, PROBE_ID, export_path=config['system_paths']['export_path'], camera_data = image_data)
                #print(json.dumps(db_json, indent=4, sort_keys=True))
                play_tone(config['tone_paths']['done'])
            else:
                success = False
                data = {
                    "camera_info": camera_data,
                    "sensor_info": sensor_data,
                    "gps_info": gps_data,
                    "probe_id": PROBE_ID
                }
            
        with sample_capture_busy.get_lock():
            sample_capture_busy.value -= 1
    else:
        success = False
        data = "Could not proccess your capture request, waiting for other captures to return."
        print(data)

    return {"success": success,
            "data": data}

@app.route("/sample/remove", methods=['GET'])
def sample_remove():
    success = None
    data = None
    batch = int(request.args.get('id'))
    success, batch_list =  get_batch_list_all()
    if success != True:
        data = "Falied to retrive batch list, plese try again."
        print(data)
    else:
        if batch in batch_list:
            success, data = remove_batch_from_db(batch)
            remove_image_files_by_batch(batch, config['system_paths']['image_path'])
            remove_json_file_by_batch(batch, config['system_paths']['export_path'])  
        else:
            success = False
            data = "The specified batch %d does not exist in the data db." %int(batch)
            print(data)
    return {"success": success,
            "data": data}

@app.route("/sample/image_list", methods=['GET'])
def sample_image_list():
    success =  None
    data = None
    batch_id = request.args.get('id')
    if str(batch_id).isnumeric():
        success, data = get_image_files_by_batch(int(batch_id), config['system_paths']['image_path'])
    else:
        success = False
        data = "Please provide a valid batch id."
    return {"success": success,
        "data": data}

@app.route("/sample/tmp_image_list", methods=['GET'])
def sample_tmp_image_list():
    success, data = get_tmp_image_files(config['system_paths']['image_path'])
    return {"success": success,
        "data": data}

@app.route("/sample/details", methods=['GET'])
def sample_details():
    data = None
    success = None
    batch_id = request.args.get('id')
    if str(batch_id).isnumeric():

        data = {}
        classification_json = get_json_by_batch(str(batch_id), config['system_paths']['export_path'])
        data = classification_json
        if len(classification_json) == 0:
            success, camera_data = get_image_files_by_batch(int(batch_id), config['system_paths']['image_path'])
            image_data =  [({'name': img}) for img in camera_data if img is not None] 
            data = generate_json_from_db(batch_id, PROBE_ID, export_path=config['system_paths']['export_path'], camera_data = image_data)
              
            success = False
            #data = ("Please provide a valid batch id.")
        else:
            success = True
    else:
        success = False
        data = ("Request is missing neccessary keys.")

    message =  {"success": success, "data": data}
    return message

@app.route("/action/<path:subpath>", methods=['GET', 'POST'])
def action(subpath):
    success = None
    data = None
    content = []

    message_list = config['actions']['message_list']
    script_list = config['actions']['script_list']
    message = None

    if request.method == "POST":
        content = request.json
        if content is None:
            success = False
            data = "Did not receive JSON object."

    if success is not False:
        if subpath in message_list:
            message = copy.copy(message_list[subpath])
            for item in content:
                if item in message:
                    message[item] = content[item]
            if None in message.values():
                success = False
                data = "Request is missing neccessary keys."
                print(data)

        elif subpath in script_list:
            print("running script")
            output = Popen([script_list[subpath]['script']], stdin=PIPE, stdout=DEVNULL, stderr=STDOUT)  
            print(output)
            success = True
            data = output
        else:
            success = False
            data = ("Request %s does not exist." %subpath)
        
        print(message)

        if success is None:
            if message['socket'] == "camera":
                success, data = camera_socket.query(message)
            elif message['socket'] == "core":
                success, data = core_socket.query(message)
            elif message['socket'] == "gps":
                success, data = gps_socket.query(message)
            elif message['socket'] == "drone":
                if check_if_running("AkerSC2"):
                    success, data = drone_socket.query(message, timeout=10000)
                else:
                    success, data = False, "This service is down."
            else:
                success = False
                data = "Invalid Socket."
        
    return {"success": success,
            "data": data}

if __name__=='__main__':
    print("HELLO")
    socketio.run(app, host="0.0.0.0", port=5000, debug=False, use_reloader=False)
    #app.run(host="0.0.0.0", port=8080, debug=True, threaded=True)