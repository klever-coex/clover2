#!/bin/bash
set -e

source /opt/ros/$ROS_DISTRO/setup.bash
source /opt/clover2/setup.bash

exec "$@"
