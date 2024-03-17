#!/bin/bash

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)
cd "$parent_path"
echo $parent_path

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


echo "Checking if ./venv/ exits. "
if [ -d "./venv" ]
then
    echo "./venv/ found skipping creation."
else
    echo "./venv/ not found creating python virtual environment."
    python3.9 -m venv venv
fi

source "venv/bin/activate"
pip3 install --upgrade pip
python3.9 setup.py install