#!/bin/bash

# test script for debugging.
# meant to work with https://github.com/Heath123/casio-emu/

# EDIT THESE PATHS TO MATCH YOUR SYSTEM
EMULATOR_PATH="/home/varun/Downloads/casio-emu-master/build/calcemu"
ADDIN_PATH="/home/varun/Desktop/test/MyAddin.g3a"
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd "$SCRIPT_DIR"

if [ ! -f "$EMULATOR_PATH" ]; then
    echo "ERROR: The casio emulator build file isn't at $EMULATOR_PATH"
    echo "Make sure to install it and edit the path"
    exit 1
fi

echo "Building..."
fxsdk build-cg
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to build for fx-CG-50"
    exit 1
fi

if [ ! -f "$ADDIN_PATH" ]; then
    echo "ERROR: Built g3a file isn't at $ADDIN_PATH"
    echo "Make sure to build using fxsdk build-cg and edit the path"
    exit 1
fi

echo "Starting emulator..."
exec "$EMULATOR_PATH" "$ADDIN_PATH"
