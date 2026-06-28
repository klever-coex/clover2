#! /usr/bin/env bash

export RCUTILS_COLORIZED_OUTPUT=1
export ROS_AUTOMATIC_DISCOVERY_RANGE=LOCALHOST

source /opt/ros/jazzy/setup.bash

if [ -f /opt/clover2/ws/install/setup.bash ]; then
    source /opt/clover2/ws/install/setup.bash

    klever5_launcher.py --config /opt/clover2/.config.yaml
else
    echo "The clover2 workspace not exists"
    exit 255
fi
