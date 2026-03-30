#!/bin/bash 

if [ "$#" -eq 0 ]; then
    mkdir -p code/cmake-build &&
    mkdir -p assignments &&
    cd code/cmake-build &&
    cmake .. &&
    make &&
    cd ../.. &&
    ./code/cmake-build/da_tool
elif [ "$#" -eq 3 ] && [ "$1" = "-b" ]; then
    mkdir -p code/cmake-build &&
    mkdir -p assignments &&
    cd code/cmake-build &&
    cmake .. &&
    make &&
    cd ../.. &&
    ./code/cmake-build/da_tool -b "dataset/input/$2.csv" "assignments/$3.csv"
else
    echo "Usage:"
    echo "  $0"
    echo "  $0 -b <input_filename> <output_filename>"
    exit 1
fi