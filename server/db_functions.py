
from flask import current_app, g
import sqlite3
import re
import os
import json
import time

def add_if_key_not_exist(dict_obj, template):
    """ Add new key-value pair to dictionary only if
    key does not exist in dictionary. """
    for key in template:
        if key not in dict_obj:
            dict_obj.update({key: template[key]})
    return dict_obj

def init_db():
    db = get_db()
    with current_app.open_resource('db/schema.sql') as f:
        db.executescript(f.read().decode('utf8'))

def make_dicts(cursor, row):
    return dict((cursor.description[idx][0], value)
                for idx, value in enumerate(row))

def get_db():
    if 'db' not in g:
        g.db = sqlite3.connect(
            current_app.config['DATABASE'],
            detect_types=sqlite3.PARSE_DECLTYPES
        )
        g.db.row_factory = make_dicts

    return g.db

def add_batch_to_db(work_order_id=None, local_batch=None, epoch=None, stress_items=None, notes=None):
    """
    Add a batch to the db.
    :param batch_info
    :return success: bool provide feedback if the request was carried out with out error
    :return message: string with details regarding the execution of the function
    """
    sql = "INSERT INTO samples VALUES(NULL," \
                                    ":work_order_id," \
                                    ":local_batch," \
                                    ":notes," \
                                    ":stress_items," \
                                    ":epoch" \
                                    ")"
    details = {
        "work_order_id": work_order_id,
        "local_batch": local_batch, 
        "notes": notes,
        "stress_items": str(stress_items),
        "epoch": epoch
    }

    if details['work_order_id'] is None or details['local_batch'] is None or details['epoch'] is None:
        success = False
        message = "One or more required batch paramaters are missing."
    else:
        db = get_db()
        cur = db.cursor()
        cur.execute(sql, details)
        db.commit()
        cur.close()
        success = True
        message = ("Batch %d was added to the db." %int(details['local_batch']))
    return success,  message

def  add_drone_data_to_db(drone_info, local_batch, epoch):
    sql = "INSERT INTO drone_readings VALUES(NULL," \
                                    ":local_batch," \
                                    ":epoch," \
                                    ":waypoint," \
                                    ":altitude_absolute," \
                                    ":latitude_deg," \
                                    ":longitude_deg," \
                                    ":ground_detected," \
                                    ":ground_altitude," \
                                    ":canopy_detected," \
                                    ":canopy_altitude," \
                                    ":tree_mode," \
                                    ":tether_length," \
                                    ":battery" \
                                    ")"
    drone_template = {
        'local_batch': local_batch,
        'epoch': epoch,
        'waypoint': None,
        'altitude_absolute': None,
        'latitude_deg': None,
        'longitude_deg':  None,
        'ground_detected': None,
        'ground_altitude': None,
        'canopy_detected': None,
        'canopy_altitude': None,
        'tree_mode': None,
        'tether_length': None,
        'battery': None
    } 
    drone_info = add_if_key_not_exist(drone_info, drone_template)
    db = get_db()
    cur = db.cursor()
    cur.execute(sql, drone_info)
    db.commit()
    cur.close()
    success = True
    message = ("Drone data for batch id %d was added to the db." %int(drone_info['local_batch']))
    return success,  message

def add_env_sensor_data_to_db(local_batch=None, temperature=None, humidity=None, pressure=None, carbon_dioxide=None, oxygen=None, internal_upper_celsius=None, grav=[None, None, None], epoch=None):
    """
    Add image to the db.
    :param batch_id: batch id the image is associated with
    :param epoch: epoch time when image was taken
    :return success: bool provide feedback if the request was carried out with out error
    :return message: string with details regarding the execution of the function
    """
    sql = "INSERT INTO external_sensor_readings VALUES(NULL," \
                                    ":tip_temperature_celsius," \
                                    ":external_humidity_percentage," \
                                    ":external_atmospheric_pressure_hectopascals," \
                                    ":carbon_dioxide_ppm," \
                                    ":oxygen_percentage," \
                                    ":internal_upper_celsius," \
                                    ":grav_x," \
                                    ":grav_y," \
                                    ":grav_z," \
                                    ":local_batch," \
                                    ":epoch" \
                                    ")"
    details = {
        'tip_temperature_celsius': temperature,
        'external_humidity_percentage': humidity,
        'external_atmospheric_pressure_hectopascals': pressure,
        'carbon_dioxide_ppm': carbon_dioxide,
        'grav_x': grav[0],
        'grav_y': grav[1],
        'grav_z': grav[2],
        'oxygen_percentage': oxygen,
        'internal_upper_celsius': internal_upper_celsius,
        'local_batch': local_batch,
        'epoch': epoch
    }
    db = get_db()
    cur = db.cursor()
    cur.execute(sql, details)
    db.commit()
    cur.close()
    success = True
    message = ("Sensor data for batch id %d was added to the db." %int(details['local_batch']))
    return success,  message

def add_gps_to_db(gps_info, local_batch, epoch):
    

    sql = "INSERT INTO gps_readings VALUES(NULL," \
                                    ":local_batch," \
                                    ":epoch," \
                                    ":sentence_identifier," \
                                    ":time," \
                                    ":latitude," \
                                    ":latitude_north_south," \
                                    ":longitude," \
                                    ":longitude_east_west," \
                                    ":fix_quality," \
                                    ":number_of_satellites," \
                                    ":hdop," \
                                    ":altitude_meters," \
                                    ":height_of_geoid_above_wgs84_ellipsoid," \
                                    ":checksum" \
                                    ")"
    gps_template = {
        'local_batch': local_batch,
        'epoch': epoch,
        'sentence_identifier': None,
        'time': None,
        'latitude': None,
        'latitude_north_south': None,
        'longitude': None,
        'longitude_east_west': None,
        'fix_quality': None,
        'number_of_satellites': None,
        'hdop': None,
        'altitude_meters': None,
        'height_of_geoid_above_wgs84_ellipsoid': None,
        'checksum': None
    }
    gps_info = add_if_key_not_exist(gps_info, gps_template)
    db = get_db()
    cur = db.cursor()
    cur.execute(sql, gps_info)
    db.commit()
    cur.close()
    success = True
    message = ("gps data for batch id %d was added to the db." %int(gps_info['local_batch']))
    return success,  message

def add_image_to_db(camera_info, local_batch, epoch):
    """
    Add image to the db.
    :param local_batch: batch id the image is associated with
    :param camera_info: dict of camera props
    :param epoch: epoch time when image was taken
    :return success: bool provide feedback if the request was carried out with out error
    :return message: string with details regarding the execution of the function
    """
    sql = "INSERT INTO images VALUES(NULL," \
                                    ":name," \
                                    ":camera_index," \
                                    ":camera_model," \
                                    ":local_batch," \
                                    ":epoch" \
                                    ")"
    camera_template = {
        "name": None,
        "camera_index": None,
        "camera_model": None,
        "local_batch": local_batch,
        "epoch": epoch
    }

    camera_info = add_if_key_not_exist(camera_info, camera_template)

    if None in camera_info.values():
        success = False
        message = "One or more required batch paramaters are missing."
    else:
        db = get_db()
        cur = db.cursor()
        cur.execute(sql, camera_info)
        db.commit()
        cur.close()
        success = True
        message = ("Image for batch id %d was added to the db." %int(camera_info['local_batch']))
    return success,  message

def add_mission_to_db(work_order_id=None, grower=None, field_name=None, crop_type=None, growth_stage=None, user=None, company=None, job=None, scout_mode=None, drone_mode=None, apn_target=None):
    """
    Add a mission to the db.
    :param mission_info
    :return success: bool provide feedback if the request was carried out with out error
    :return message: string with details regarding the execution of the function
    """
    sql = "INSERT INTO mission VALUES(NULL," \
                                    ":work_order_id," \
                                    ":grower,"\
                                    ":field_name," \
                                    ":crop_type," \
                                    ":growth_stage," \
                                    ":user," \
                                    ":company," \
                                    ":job," \
                                    ":scout_mode," \
                                    ":drone_mode," \
                                    ":apn_target," \
                                    ":downloaded," \
                                    ":downloaded_epoch" \
                                    ")"
    details = {
        "work_order_id": work_order_id,
        "grower": grower,
        "field_name": field_name,
        "crop_type": crop_type,
        "growth_stage": growth_stage,
        "user": user,
        "company": company,
        "job": job,
        "scout_mode": scout_mode,
        "drone_mode": drone_mode,
        "apn_target": apn_target,
        "downloaded": 0,
        "downloaded_epoch": 0
    }
    if None in details.values():
        success = False
        message = "One or more required mission paramaters are missing."
    else:
        db = get_db()
        cur = db.cursor()
        cur.execute(sql, details)
        db.commit()
        cur.close()
        success = True
        message = ("Mission %d was added to the db." %int(details['work_order_id']))
    return success,  message

def generate_json_from_db(local_batch, probe_id, export_path=None, camera_data=None):
    jdict = {}
    cycle = {}
    jdict['cycles'] = []
    batch_info = get_batch_details(local_batch)

    jdict.update(batch_info)
    jdict["json_version"] = 2
    
    cycle['images'] = camera_data

    cycle['drone'] = get_drone_data_by_batch_id(local_batch)
    cycle['gps'] = get_gps_data_by_batch_id(local_batch)

    cycle['environmentals'] = get_env_sensor_data_by_batch_id(local_batch)

    _, mission_info = get_mission_details(batch_info['work_order_id'])
    jdict['user'] = {
            "user": mission_info['user'],
            "company": mission_info['company'],
            "job": mission_info['job']
    }
    jdict['field'] = {
            "grower":mission_info['grower'],
            "field_name":mission_info['field_name'],
            "crop_type": { 
                "name":mission_info['crop_type']
            },
            "growth_stage":{
                "text":mission_info['growth_stage']
            }
    }

    jdict['system_settings'] = {
        "scout_mode": mission_info['scout_mode'],
        "drone_mode": mission_info['drone_mode'],
        "apn_target": mission_info['apn_target'],
        "probe_serial": int(probe_id)
    }
    jdict['cycles'].append(cycle)

    if export_path is not None:
        base_name = f"{local_batch}-{batch_info['epoch']}-{probe_id}-"
        jsonName = os.path.join(export_path, (base_name + "classification.json"))
        with open(jsonName, "w") as jout:
            json.dump(jdict, jout, indent=4)

    return jdict

def get_batch_details(id=None):
    """
    Retrive details of a batch based on id
    :param id: bathc id number of the task
    :return batch_list: List of batches
    """
    message = None
    if id is not None:
        sql = "SELECT * FROM samples WHERE local_batch=?"
        db = get_db()
        db.row_factory = sqlite3.Row
        cur = db.cursor()
        cur.execute(sql, (id,))
        data = cur.fetchone()
        cur.close()
        if data is not None:
            message = dict(data)
            message.pop('id', None)
    return message

def get_batch_list_all():
    """
    Retrive a list of all batches from the db
    :return success: bool
    :return batch_list: List of batches
    """
    sql = "SELECT * FROM samples"
    db = get_db()
    cur = db.cursor()
    cur.execute(sql)
    data_list = cur.fetchall()
    cur.close()
    batch_list = []
    for entry in data_list:
        batch_list.append(entry['local_batch'])
    success = True
    message = batch_list
    return success, message

def get_batch_list_for_wo(wo=None):
    """
    Retrive list of batches from the db
    :param wo: workorder number of the task
    :return success: bool
    :return batch_list: List of batches
    """
    if isinstance(wo, int):
        sql = "SELECT * FROM samples WHERE work_order_id=?"
        db = get_db()
        cur = db.cursor()
        cur.execute(sql, (wo,))
        data_list = cur.fetchall()
        cur.close()
        batch_list = []
        for entry in data_list:
            batch_list.append(entry['local_batch'])
        success = True
        message = batch_list
    else:
        success = False
        message = "Workorder number is invalid."
    return success, message

def get_gps_data_by_batch_id(local_batch=None):
    data = None
    if local_batch is not None:
        sql = "SELECT * FROM gps_readings WHERE local_batch=?" 
        db = get_db()
        db.row_factory = sqlite3.Row
        cur = db.cursor()
        cur.execute(sql, (local_batch,))
        raw_data = cur.fetchone()
        cur.close()
        if raw_data is not None:
            data = dict(raw_data)
            data.pop('id', None)
            data.pop('epoch', None)
            if data['latitude'] is None or data['longitude'] is None:
                data = {}
        else:
            print("The function: get_gps_data_by_batch_id failed to execute due to the provided batch id does not exist in the db..")
    else:
        print("The function: get_gps_data_by_batch_id failed to execute due to invalid input.")
    return data

def get_drone_data_by_batch_id(local_batch=None):
    data = None
    if local_batch is not None:
        sql = "SELECT * FROM drone_readings WHERE local_batch=?" 
        db = get_db()
        db.row_factory = sqlite3.Row
        cur = db.cursor()
        cur.execute(sql, (local_batch,))
        raw_data = cur.fetchone()
        cur.close()
        if raw_data is not None:
            data = dict(raw_data)
            data.pop('id', None)
            data.pop('local_batch', None)
            data.pop('epoch', None)
        else:
            data = {}
            print("The function: get_drone_data_by_batch_id failed to execute due to the provided batch id does not exist in the db..")
    else:
        print("The function: get_drone_data_by_batch_id failed to execute due to invalid input.")
    return data

def get_env_sensor_data_by_batch_id(local_batch=None):
    """
    Get Env data form the db.
    :param local_batch: batch id the image is associated with
    :return success: bool provide feedback if the request was carried out with out error
    :return message: string with details regarding the execution of the function
    """
    data = None
    if local_batch is not None:
        sql = "SELECT * FROM external_sensor_readings WHERE local_batch=?" 
        db = get_db()
        db.row_factory = sqlite3.Row
        cur = db.cursor()
        cur.execute(sql, (local_batch,))
        raw_data = cur.fetchone()
        cur.close()
        if raw_data is not None:
            data = dict(raw_data)
            data.pop('id', None)
            data.pop('epoch', None)
            data.pop('local_batch', None)
        else:
            print("The function: get_env_sensor_data_by_batch_id failed to execute due to the provided batch id %s does not exist in the db." %str(local_batch))
    else:
        print("The function: get_env_sensor_data_by_batch_id failed to execute due to invalid input.")
    return data

def get_images_by_batch_id_from_db(id):
    success = None
    message = None
    if isinstance(id, int):
        sql = "SELECT * FROM images WHERE local_batch=?"
        db = get_db()
        db.row_factory = sqlite3.Row
        cur = db.cursor()
        cur.execute(sql, (id,))
        data = cur.fetchall()
        cur.close()
        if len(data) > 0:
            success = True
            message = []
            for camera in data:
                camera_dict = dict(camera)
                camera_dict.pop('epoch', None)
                camera_dict.pop('local_batch', None)
                message.append(camera_dict)
        else:
            success = False
            message = ("The provided batch ID %d number does not exist in the db." %id)
    else:
        success = False
        message = ("The batch ID %d number is invalid." %id)
    return success, message

def get_mission_details(wo):
    success = None
    message = None
    if wo is not None:
        db = get_db()
        cur = db.cursor().execute("SELECT * FROM mission WHERE work_order_id=?", (wo,))
        data = cur.fetchone()
        cur.close()
        if data is not None:
            success = True
            message = dict(data)
        else:
            success = False
            message = "The provided workorder number does not exist in the db."
    else:
        success = False
        message = "Workorder number is invalid."
    return success, message

def get_mission_downloaded_status(wo):
    success = None
    message = None
    if wo is not None:
        db = get_db()
        cur = db.cursor().execute("SELECT * FROM mission WHERE work_order_id=?", (wo,))
        data = cur.fetchone()
        cur.close()
        if data is not None:
            success = True
            mission_dict = dict(data)
            message = {"state":mission_dict['downloaded'],
                        "epoch": mission_dict['downloaded_epoch']}
        else:
            success = False
            message = "The provided workorder number does not exist in the db."
    else:
        success = False
        message = "Workorder number is invalid."
    return success, message

def get_workorder_list():
    """
    Retrive list of workorder from the db
    "return success: bool  
    :return wo_list: List of work orders
    """
    sql = "SELECT work_order_id FROM mission"
    db = get_db()
    cur = db.cursor()
    cur.execute(sql)
    data_list = cur.fetchall()
    cur.close()
    wo_list = []
    for entry in data_list:
        wo_list.append(entry['work_order_id'])
    success = True
    return success, wo_list

def remove_image_by_image_id_from_db(id=None):
    """
    Remove images by id
    :param id: id number of the images in the db
    :return success: bool
    :return message: string
    """
    if str(id).isnumeric():          
        sql = 'DELETE FROM images WHERE id=?'
        db = get_db()
        cur = db.cursor()
        cur.execute(sql, (id,))
        db.commit()
        cur.close()
        success = True
        message = ("image with id was %d removed from the db." %int(id))
    else:
        success = False
        message = "image id number is invalid."
    return success, message

def remove_batch_from_db(id=None):
    """
    Remove a batch by id
    :param id: id number of the batch
    :return success: bool
    :return message: string
    """
    if str(id).isnumeric():
        sql = 'DELETE FROM samples WHERE local_batch=?'
        db = get_db()
        cur = db.cursor()
        cur.execute(sql, (id,))
        db.commit()
        cur.close()
        success = True
        message = ("Batch %d was removed from the db." %int(id))
    else:
        success = False
        message = "Batch id number is invalid."
    return success, message

def remove_mission_from_db(wo=None):
    """
    Remove a mission by wo
    :param wo: workorder number of the task
    :return success: bool
    :return message: string
    """
    if wo is not None:
        sql = 'DELETE FROM mission WHERE work_order_id=?'
        db = get_db()
        cur = db.cursor()
        cur.execute(sql, (wo,))
        db.commit()
        cur.close()
        success = True
        message = ("Workorder %d was removed from the db." %int(wo))
    else:
        success = False
        message = "Workorder number is invalid."
    return success, message

def update_mission_details(d):
    success = None
    message = None
    db = get_db()
    cur = db.cursor()

    sql = """Update mission set 
            grower = ?, 
            field_name = ?,
            crop_type = ?,
            growth_stage = ?,
            user = ?,
            company = ?,
            job = ?,
            scout_mode = ?,
            drone_mode = ?,
            apn_target = ?
            where 
            work_order_id = ?"""
    cur.execute(sql, (d['grower'],
                        d['field_name'], 
                        d['crop_type'], 
                        d['growth_stage'], 
                        d['user'], 
                        d['company'], 
                        d['job'], 
                        d['scout_mode'], 
                        d['drone_mode'], 
                        d['apn_target'], 
                        d['work_order_id']))
    db.commit()
    cur.close()
    success = True
    message = "Database updated successfuly with new mission details."
    return success, message

def update_mission_downloaded(wo, downloaded):
    success = None
    message = None
    db = get_db()
    cur = db.cursor()

    sql = """Update mission set 
            downloaded = ?,
            downloaded_epoch = ?
            where 
            work_order_id = ?"""

    epoch = int(time.time()) if bool(downloaded) else None 
    cur.execute(sql, (bool(downloaded), epoch, wo))
    db.commit()
    cur.close()
    success = True
    message = "Database updated successfuly with new mission downloaded state."
    return success, message

def verify_workorder_number(wo):
    result = False
    if wo is not None:
        if str(wo).isnumeric():
            if int(wo) > 0:
                _ , work_order_list = get_workorder_list()
                if int(wo) in work_order_list:
                    result = True
    return result

