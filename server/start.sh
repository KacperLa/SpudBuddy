#!/bin/bash

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)
cd "$parent_path"
echo $parent_path

f="run --host=0.0.0.0 --port=5000"

if [ "$#" -eq "0" ] 
then
        echo "No arguments supplied."
else
    while getopts c:f: flag
    do
        case "${flag}" in
            f) f=${OPTARG};;
        esac
    done
fi

config_json_file="./flask_config.json"

echo $config_json_file
echo $f

source "venv/bin/activate"
# export FLASK_ENV=development
export FLASK_APP=main
export CONFIG_JSON_FILE=$config_json_file

python main.py $f
#flask $f