import argparse
import csv
import glob
import json
import os
import re
from os.path import exists
import time
import pathlib


def c_to_f(c):
    return (9/5 * c) + 32

def f_to_c(f):
    return (f - 32) * 5/9

def bme280_hash(basename=None, batch_id=None, dir_path=None):
    info = {}
    info['farenheit'] = None
    info['celsius'] = None
    info['hectopascals'] = None
    info['humidity'] = None

    file_glob = None

    if basename is not None:
        file_glob = glob.glob(os.path.join(dir_path, (os.path.join(dir_path, f"{basename}-BME280.csv"))))
    elif batch_id is not None and dir_path is not None:
        file_glob = glob.glob(os.path.join(dir_path, (batch_id + "-*-BME280.csv")))
    
    if len(file_glob) > 0:
        file_path = file_glob[0]
        with open(file_path, 'r') as csvfile:  
            line =  csv.DictReader(csvfile) 
            if line.fieldnames is not None:
                info['celsius'] = float(line.fieldnames[1])
                info['hectopascals'] = float(line.fieldnames[2])
                info['humidity'] = float(line.fieldnames[3])
                
    return info
    
def drone_hash(basename=None, batch_id=None, dir_path=None):
    info = {}

    file_glob = None

    if basename is not None:
        file_glob = glob.glob(os.path.join(dir_path, (os.path.join(dir_path, f"{basename}-DRONE.csv"))))
    elif batch_id is not None and dir_path is not None:
        file_glob = glob.glob(os.path.join(dir_path, (batch_id + "-*-DRONE.csv")))
    
    if len(file_glob) > 0:
        file_path = file_glob[0]
        with open(file_path, 'r') as csvfile:  
            line =  csv.DictReader(csvfile) 
            if line.fieldnames is not None:
                info['waypoint'] = int(line.fieldnames[1].strip())
                info['altitude_absolute'] = float(line.fieldnames[2].strip())
                info['latitude_deg'] = float(line.fieldnames[4].strip())
                info['longitude_deg'] = float(line.fieldnames[3].strip())
                info['canopy_detected'] = bool(line.fieldnames[5].strip())
                info['canopy_altitude'] = float(line.fieldnames[6].strip())
                info['ground_detected'] = bool(line.fieldnames[7].strip())
                info['ground_altitude'] = float(line.fieldnames[8].strip())
                info['tree_mode'] = bool(line.fieldnames[9].strip())
                info['tether_length'] = float(line.fieldnames[10].strip())

    return info

def temphum_hash(basename=None, batch_id=None, dir_path=None):
    info = {}
    info['farenheit'] = None
    info['celsius'] = None
    info['humidity'] = None

    file_glob = None

    if basename is not None:
        file_glob = glob.glob(os.path.join(dir_path, (os.path.join(dir_path, f"{basename}-TEMP.csv"))))
    elif batch_id is not None and dir_path is not None:
        file_glob = glob.glob(os.path.join(dir_path, (batch_id + "-*-TEMP.csv")))
    
    if len(file_glob) > 0:
        file_path = file_glob[0]
        with open(file_path, 'r') as csvfile:
            line =  csv.DictReader(csvfile) 
            if line.fieldnames is not None:
                info['farenheit'] = c_to_f(float(line.fieldnames[1]))
                info['celsius'] = float(line.fieldnames[1])
                info['humidity'] = float(line.fieldnames[2])

    return info
        
def co2_hash(basename=None, batch_id=None, dir_path=None):
    info = {}

    info['ppm'] = None

    file_glob = None
    if basename is not None:
        file_glob = glob.glob(os.path.join(dir_path, (os.path.join(dir_path, f"{basename}-CO2.csv"))))
    elif batch_id is not None and dir_path is not None:
        file_glob = glob.glob(os.path.join(dir_path, (batch_id + "-*-CO2.csv")))
    
    if len(file_glob) > 0:
        file_path = file_glob[0]
        with open(file_path, 'r') as csvfile:  
            line =  csv.DictReader(csvfile)
            if line.fieldnames is not None:
                info['ppm'] = int(line.fieldnames[1])

    return info
    
def o2_hash(basename=None, batch_id=None, dir_path=None):
    info = {}
    info['percent'] = None

    file_glob = None

    if basename is not None:
        file_glob = glob.glob(os.path.join(dir_path, (os.path.join(dir_path, f"{basename}-O2.csv"))))
    elif batch_id is not None and dir_path is not None:
        file_glob = glob.glob(os.path.join(dir_path, (batch_id + "-*-O2.csv")))
    
    if len(file_glob) > 0:
        file_path = file_glob[0]
        with open(file_path, 'r') as csvfile:  
            line =  csv.DictReader(csvfile)
            if line.fieldnames is not None:
                info['percent'] =float(line.fieldnames[1])
    
    return info

def bmo055_hash(basename=None, batch_id=None, dir_path=None):
    info = {}
    info['grav_x'] = None
    info['grav_y'] = None
    info['grav_z'] = None

    file_path = None

    if basename is not None:
        file_name = os.path.join(dir_path, f"{basename}-BNO055.csv")
        file_glob = glob.glob(file_name)
        if len(file_glob) > 0:
            file_path = file_glob[0]
    elif batch_id is not None and dir_path is not None:
        file_glob = glob.glob(os.path.join(dir_path, (batch_id + "-*-BNO055.csv")))
        if len(file_glob) > 0:
            file_path = file_glob[0]
    
    if file_path is not None:
        with open(file_path, 'r') as csvfile:  
            line =  csv.DictReader(csvfile)
            if line.fieldnames is not None:
                info['grav_x'] = float(line.fieldnames[7])
                info['grav_y'] = float(line.fieldnames[8])
                info['grav_z'] = float(line.fieldnames[9])
            
    return info

def gps_hash(basename, dir_path=None):
    info = {}

    file_path = None

    filename = f"{basename}-GPS.csv"
    file_glob = glob.glob(os.path.join(dir_path, filename))
    if len(file_glob) > 0:
        file_path = file_glob[0]
        with open(os.path.join(dir_path, filename), 'r') as csvfile:  
            line =  csv.DictReader(csvfile) 
            if line.fieldnames is not None:
                info['sentence_identifier'] = line.fieldnames[1]     
                info['time'] = line.fieldnames[2]
                info['latitude'] = float(line.fieldnames[3])
                info['latitude_north_south'] = line.fieldnames[4]
                info['longitude'] = float(line.fieldnames[5])
                info['longitude_east_west'] = line.fieldnames[6]
                info['fix_quality'] = int(line.fieldnames[7])
                info['number_of_satellites'] = int(line.fieldnames[8])
                info['hdop'] = line.fieldnames[9]
                info['altitude_meters'] = float(line.fieldnames[10])
                info['height_of_geoid_above_wgs84_ellipsoid'] = line.fieldnames[12] + "," + line.fieldnames[13]
                info['checksum'] = line.fieldnames[15]
                
    return info

def get_image_files_by_batch(batch_id, dirName):
    image_list = []
    has_images = False
    if str(batch_id).isnumeric():
        image_paths = glob.glob(os.path.join(dirName, (str(batch_id) + "-*[0-9].jpg")))
        if len(image_paths) > 0:
            has_images = True
            for img in image_paths:
                image_list.append(os.path.basename(img))
    return has_images, image_list

def get_tmp_image_files(dirName):
    image_list = []
    has_images = False
    image_paths = glob.glob(os.path.join(dirName, "*tmp-*[0-9].jpg"))
    if len(image_paths) > 0:
        has_images = True
        for img in image_paths:
            image_list.append(os.path.basename(img))
    print("IMG LIST", image_list)
    return has_images, image_list

def get_mission_log_files(dirName):
    file_list = []
    has_files = False
    file_paths = glob.glob(os.path.join(dirName, "mission-*.txt"))
    if len(file_paths) > 0:
        has_file = True
        for img in file_paths:
            file_list.append(os.path.basename(img))
    print("LOG LIST", file_list)
    return has_files, file_list

def remove_image_files_by_batch(batch_id, dirName):
    if str(batch_id).isnumeric():
        image_paths = glob.glob(os.path.join(dirName, (str(batch_id) + "-*[0-9].jpg")))
        if len(image_paths) > 0:
            for img in image_paths:
                os.remove(img)
                print("%s removed from image dir." %img)

def remove_json_file_by_batch(batch_id, dirName):
    if str(batch_id).isnumeric():
        image_paths = glob.glob(os.path.join(dirName, (str(batch_id) + "-*.json")))
        if len(image_paths) > 0:
            for img in image_paths:
                os.remove(img)
                print("%s removed for export dir." %img)

def get_json_by_batch(batch_id, dirName=None):
    info = []
    if batch_id is not None:
        json_path = glob.glob(os.path.join(dirName, (batch_id + "-*-classification.json")))
        print("json_path", json_path)
        if len(json_path) > 0:
            with open(json_path[0], "r") as json_in:
                info = json.load(json_in)
    return info

def get_json_name_by_batch(batch_id, dirName=None):
    info = None
    if batch_id is not None:
        json_path = glob.glob(os.path.join(dirName, (batch_id + "-*-classification.json")))
        if len(json_path) > 0:
            info = os.path.basename(json_path[0])
    return info

def get_csv_name_by_batch(batch_id, dirName=None):
    info = []
    if batch_id is not None:
        csv_paths = glob.glob(os.path.join(dirName, (batch_id + "-*.csv")))
        if len(csv_paths) > 0:
            for csv in csv_paths:
                info.append(os.path.basename(csv))
    return info

def generate_json(basename=None, cycle_id=None, export_path=None, image_path=None, config=None, imgs=[]):
    basename += str(cycle_id)
    batch_id, epoch, probe_id, cycles,cycle_id = basename.split('-')
    
    jdict = {}
    jdict['cycles'] = []
    jdict['epoch'] = int(epoch)
    jdict['local_batch'] = int(batch_id)
    jdict['notes'] = ""
    jdict['stress_items'] = []

    cycle = {}
    cycle['number'] = cycle_id
    cycle['drone'] = drone_hash(basename=basename, dir_path=export_path)
    cycle['gps'] = gps_hash(basename=basename, dir_path=export_path)

    bme = bme280_hash(basename=basename, dir_path=export_path)
    temp = temphum_hash(basename=basename, dir_path=export_path)
    co2 = co2_hash(basename=basename, dir_path=export_path)
    o2 = o2_hash(basename=basename, dir_path=export_path)
    bno = bmo055_hash(basename=basename, dir_path=export_path)

    cycle['environmentals'] = {}
    cycle['environmentals']['tip_temperature_celsius'] = temp['celsius']
    cycle['environmentals']['internal_upper_celsius'] = bme['celsius']
    cycle['environmentals']['external_humidity_percentage'] = temp['humidity'] 
    cycle['environmentals']['external_atmospheric_pressure_hectopascals'] = bme['hectopascals']
    cycle['environmentals']['carbon_dioxide_ppm'] = co2['ppm']
    cycle['environmentals']['oxygen_percent'] = o2['percent']
    cycle['environmentals']['grav_x'] = bno['grav_x']
    cycle['environmentals']['grav_y'] = bno['grav_y']
    cycle['environmentals']['grav_z'] = bno['grav_z']
    jdict['cycles'].append(cycle)

    cycle['images'] = []
    for i in imgs:
        if i is not None:
            _image = {}
            _image['name'] = i
            _image['lux'] = -1
            cycle['images'].append(_image)

    jdict['user'] = {
            "user": config['user'],
            "company": config['company'],
            "job": config['job']
    }
    jdict['field'] = {
            "grower": config['grower'],
            "field_name": config['field_name'],
            "crop_type": config['crop_type'],
            "growth_stage": config['growth_stage']
    }

    jdict['work_order_id'] = config['work_order_id']

    jdict['system_settings'] = {
        "scout_mode": config['scout_mode'],
        "drone_mode": config['drone_mode'],
        "apn_target": config['apn_target'],
        "probe_serial": int(probe_id)
    }
    
    jsonName = os.path.join(export_path, re.sub('-1$', '', basename) + "-classification.json")
    with open(jsonName, "w") as jout:
        json.dump(jdict, jout, indent=4)

def snip_sensor_files(sensor_paths, basename, cycle_id, export_path):
    basename += str(cycle_id)
    success = False
    snip_log = []
    for sensor in sensor_paths:
        path_to_log = sensor_paths[sensor]
        if os.path.isfile(path_to_log):
            if (time.time() - pathlib.Path(path_to_log).stat().st_mtime < 100):
                try:
                    data = None
                    with open(path_to_log, "r") as log_file:
                        data = log_file.readlines()[-1].rstrip()
                    with open(os.path.join(export_path, (basename+"-"+sensor+".csv")), "w") as snip_file:
                        snip_file.write(data)
                except Exception as e:
                    snip_log.append("Error%s log file." %sensor)
                    print(e)
            else:
                snip_log.append("Error %s log file too old." %sensor)
        else:
            snip_log.append("Error %s log file does not exist." %path_to_log)
    return success, snip_log

def increment_batch_id(path_to_batch_id):
    success = False
    batch_log = []
    batch_log.append("")
    batch_id = None

    with open(path_to_batch_id, "r") as batch_id_file:
        batch_id = int(batch_id_file.readlines()[0].rstrip())
    batch_id += 1
    with open(path_to_batch_id, "w") as batch_id_file:
        batch_id_file.write(str(batch_id))

    return success, int(batch_id)

if __name__ == "__main__":
    #generate_json()
    batch_id = "22"
    basename = "69-1634085714-2063-1-1"
    image_dir = "/mnt/SD_CARD/aker-images"
    export_dir = "/mnt/SD_CARD/aker-export"
    snip_sensor_files(basename, export_dir)
    #process_files(basename=basename, export_path=export_dir, image_path=image_dir, info=info, config=config)