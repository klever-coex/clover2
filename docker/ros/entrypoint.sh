#!/bin/bash
set -e

# setup ros2 environment
source "/opt/ros/$ROS_DISTRO/setup.bash"

# setup TFM workspace environment if it exists
source "/opt/tfm_ws/setup.bash"

exec "$@"
