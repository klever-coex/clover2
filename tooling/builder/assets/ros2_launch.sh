#! /usr/bin/env bash

export RCUTILS_COLORIZED_OUTPUT=1
export ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST

source /opt/ros/jazzy/setup.bash

if [ -f "$FILE" ]; then
    source /opt/clover2/ws/install/setup.bash

    ros2 launch clover2_bringup clover2.launch.py
else
    echo "The clover2 workspace not exists"
    exit 255
fi
