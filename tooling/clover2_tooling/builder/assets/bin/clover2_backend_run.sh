#! /usr/bin/env bash

. /opt/clover2/.ros2.env

source /opt/ros/jazzy/setup.bash

if [ -f /opt/clover2/ws/install/setup.bash ]; then
    source /opt/clover2/ws/install/setup.bash

    ros2 launch clover2_http web_support.launch.xml
else
    echo "The clover2 workspace not exists"
    exit 255
fi
