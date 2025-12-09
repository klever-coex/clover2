FROM ros:jazzy-ros-base

WORKDIR /clover2_ws
COPY . /clover2_ws/src

RUN apt-get update -y && \
    rosdep install --from-paths src --ignore-src -r -y && \
    apt-get install -y \
        geographiclib-tools && \
    rm -rf /var/lib/apt/lists/*

RUN wget -qO- https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh | sudo bash

RUN apt-get update -y && \
    apt-get install -y \
        ros-jazzy-mavros-msgs \
        ros-jazzy-mavros \
        ros-jazzy-mavros-extras \
        ros-jazzy-camera-info-manager \
        ros-jazzy-cv-bridge \
        ros-jazzy-image-view \
        ros-jazzy-image-geometry && \
    rm -rf /var/lib/apt/lists/*

RUN /bin/bash -c "source /opt/ros/jazzy/setup.bash && colcon build --install-base /opt/clover2 --merge-install"

COPY docker/ros/entrypoint.sh /

ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
