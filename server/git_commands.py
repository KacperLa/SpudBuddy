#!venv/bin/python3

import argparse
import csv
import glob
import json
import os
import re
from os.path import exists
import time
import pathlib
from subprocess import Popen, PIPE, STDOUT, check_output

DEVNULL = open(os.devnull, 'w')

def get_git_tag_list():
    print("Running get git tags")
    fetch_cmd = ("git fetch --all --tags").split(" ")
    print(check_output(fetch_cmd).decode('utf-8').strip())
    cmd = ("git tag -l 3*").split(" ")
    output = check_output(cmd).decode('utf-8').strip()
    tag_list = list(map(lambda x: x.strip(), output.split('\n')))
    output = {
        "data": tag_list,
        "success": True
    }
    return output

def git_checkout_tag(config, req_tag):
    print("Updating probe")
    output = check_output([config.get('system_paths', {}).get('update_probe'), '-s', config.get('SYSTEM', 'NONE'), '-f', 'update_to_tag', '-t', str(req_tag)])
    print(output.decode('utf-8'))
    data = {
        "data": "HELLO",
        "success": True
    }
    return data


if __name__ == "__main__":
   tag_json = get_git_tag_list();
   print(tag_json)