ARG BASE_IMAGE=ros:jazzy-ros-base
ARG SHORT_NAME="short"
ARG ROS_DISTRO=jazzy
ARG TARGETPLATFORM

FROM ${BASE_IMAGE} AS rosdep-depend
ARG ROS_DISTRO
COPY tooling/docker/* /clover2/

# WORKDIR /clover2_ws
# COPY . /clover2_ws/src/clover2
WORKDIR /ws
COPY clover2_sim/clover2_gz_sim/package.xml /ws/src/clover2_sim/clover2_gz_sim/package.xml
COPY clover2_sim/clover2_sim/package.xml /ws/src/clover2_sim/clover2_sim/package.xml
COPY clover2_sim/clover2_description/package.xml /ws/src/clover2_sim/clover2_description/package.xml
COPY clover2/package.xml /ws/src/clover2/package.xml
COPY clover2_common/package.xml /ws/src/clover2_common/package.xml
COPY clover2_nav/clover2_cam_feature/package.xml /ws/src/clover2_nav/clover2_cam_feature/package.xml
COPY clover2_nav/clover2_pose_msgs/package.xml /ws/src/clover2_nav/clover2_pose_msgs/package.xml
COPY clover2_nav/clover2_optical_flow/package.xml /ws/src/clover2_nav/clover2_optical_flow/package.xml
COPY clover2_nav/clover2_map/package.xml /ws/src/clover2_nav/clover2_map/package.xml
COPY clover2_nav/clover2_aruco/package.xml /ws/src/clover2_nav/clover2_aruco/package.xml
COPY clover2_fcu_bridge_msgs/package.xml /ws/src/clover2_fcu_bridge_msgs/package.xml
COPY clover2_fcu_bridge/package.xml /ws/src/clover2_fcu_bridge/package.xml

RUN /clover2/rosdep-parse.sh /ws/src $ROS_DISTRO --dependency-types=exec > /clover2/apt-depends.txt && \
    cat /clover2/apt-depends.txt

FROM ${BASE_IMAGE} AS core
ARG ROS_DISTRO
ARG TARGETPLATFORM
COPY tooling/docker/* /clover2/

WORKDIR /clover2
COPY --from=rosdep-depend /clover2/apt-depends.txt /tmp/apt-depends.txt

RUN --mount=type=cache,id=apt-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    apt-get update -y && \
    apt-get install -y \
        geographiclib-tools && \
    /clover2/cleanup-apt.sh

RUN wget -qO- https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh | sudo bash

RUN --mount=type=cache,id=apt-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    apt-get update -y && \
    apt-get install -y \
        ros-${ROS_DISTRO}-mavros-msgs \
        ros-${ROS_DISTRO}-mavros \
        ros-${ROS_DISTRO}-mavros-extras \
        ros-${ROS_DISTRO}-camera-info-manager \
        ros-${ROS_DISTRO}-cv-bridge \
        ros-${ROS_DISTRO}-image-view \
        ros-${ROS_DISTRO}-image-geometry && \
    /clover2/cleanup-apt.sh

RUN --mount=type=cache,id=apt-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    apt-get update -y && \
    cat /tmp/apt-depends.txt | xargs apt-get install -y --no-install-recommends && \
    /clover2/cleanup-apt.sh

FROM core AS source

WORKDIR /clover2_ws
COPY . /clover2_ws/src/clover2

FROM source AS builder
ARG SHORT_NAME
ARG ROS_DISTRO
ARG TARGETPLATFORM
ENV CCACHE_DIR="/root/.ccache"

RUN --mount=type=cache,id=apt-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    apt-get update -y && \
    apt-get install -y \
        ccache && \
    /clover2/cleanup-apt.sh

RUN --mount=type=cache,id=ccache-${SHORT_NAME}-${TARGETPLATFORM},target=${CCACHE_DIR} \
    --mount=type=cache,id=ros-install-${SHORT_NAME}-${TARGETPLATFORM},target=/opt/clover2 \
    --mount=type=cache,id=ros-build-${SHORT_NAME}-${TARGETPLATFORM},target=/clover2_ws/build \
    src/clover2/docker/ros/build.sh ${CCACHE_DIR} /opt/clover2 /clover2_ws/build

FROM core AS clover2-ros

WORKDIR /clover2_ws
COPY --from=builder /opt/clover2 /opt/clover2

COPY docker/ros/entrypoint.sh  /

ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]

FROM source AS clover2-sim
ARG ROS_DISTRO
ARG TARGETPLATFORM

RUN --mount=type=cache,id=apt-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    apt-get update -y && \
    apt-get install -y --no-install-recommends \
        ros-${ROS_DISTRO}-desktop=0.11.0-1* \
        ros-${ROS_DISTRO}-simulation=0.11.0-1* && \
    /clover2/cleanup-apt.sh

WORKDIR /clover2_ws
COPY --from=builder /opt/clover2 /opt/clover2

COPY docker/ros/entrypoint.sh  /

ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
