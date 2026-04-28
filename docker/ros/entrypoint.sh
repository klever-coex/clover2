#!/usr/bin/env bash
set -e

USER_ID=${LOCAL_UID}
USER_NAME=${LOCAL_USER}
GROUP_ID=${LOCAL_GID}
GROUP_NAME=${LOCAL_GROUP}

if [[ -z $USER_ID || -z $USER_NAME || -z $GROUP_ID || -z $GROUP_NAME ]]; then
    source "/opt/ros/$ROS_DISTRO/setup.bash"
    source /opt/clover2/install/setup.bash
    exec ros2 launch clover2 clover2.launch.py "$@"
else
    echo "Starting with user: $USER_NAME >> UID $USER_ID, GID: $GROUP_ID"

    groupadd -g "$GROUP_ID" "$GROUP_NAME"
    useradd -u "$USER_ID" -g "$GROUP_ID" -s /bin/bash -m -d /home/"$USER_NAME" "$USER_NAME"

    echo "$USER_NAME ALL=(ALL) NOPASSWD:ALL" >>/etc/sudoers

    source "/opt/ros/$ROS_DISTRO/setup.bash"
    source /opt/clover2/install/setup.bash

    exec /usr/sbin/gosu "$USER_NAME" "$@"
fi
