#! /usr/bin/env bash

source /opt/ros/jazzy/setup.bash
source /home/pi/clover2_ws/install/setup.bash

export RCUTILS_COLORIZED_OUTPUT=1

ros2 launch clover2 clover2.launch.py
