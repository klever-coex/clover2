ARG BASE_IMAGE=ros:jazzy-ros-base
ARG CACHE_EPOCH="unknown"
ARG ROS_DISTRO=jazzy
ARG TARGETPLATFORM

FROM ${BASE_IMAGE} AS rosdep-depend
ARG ROS_DISTRO
COPY tooling/docker/* /clover2/

WORKDIR /ws

COPY --parents ./**/package.xml /ws/src

RUN rosdep update
RUN /clover2/rosdep-parse.sh /ws/src $ROS_DISTRO --dependency-types=exec > /clover2/apt-depends.txt && \
    cat /clover2/apt-depends.txt

FROM ${BASE_IMAGE} AS core
ARG ROS_DISTRO
ARG TARGETPLATFORM
COPY tooling/docker/* /clover2/

WORKDIR /clover2
COPY --from=rosdep-depend /clover2/apt-depends.txt /tmp/apt-depends.txt

RUN --mount=type=cache,id=apt-ros-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=apt-ros-${TARGETPLATFORM},target=/var/lib/apt/lists,sharing=locked \
    apt-get update -y && \
    apt-get install -y \
        gosu \
        sudo \
        geographiclib-tools

RUN wget -qO- https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh | sudo bash

RUN --mount=type=cache,id=apt-ros-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=apt-ros-${TARGETPLATFORM},target=/var/lib/apt/lists,sharing=locked \
    apt-get update -y && \
    apt-get install -y \
        ros-${ROS_DISTRO}-camera-info-manager \
        ros-${ROS_DISTRO}-cv-bridge \
        ros-${ROS_DISTRO}-image-geometry \
        ros-${ROS_DISTRO}-image-view \
        ros-${ROS_DISTRO}-mavros \
        ros-${ROS_DISTRO}-mavros-extras \
        ros-${ROS_DISTRO}-mavros-msgs \
        ros-${ROS_DISTRO}-tf-transformations \
        ros-${ROS_DISTRO}-web-video-server

RUN --mount=type=cache,id=apt-ros-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=apt-ros-${TARGETPLATFORM},target=/var/lib/apt/lists,sharing=locked \
    apt-get update -y && \
    cat /tmp/apt-depends.txt | xargs apt-get install -y --no-install-recommends

FROM core AS source

WORKDIR /clover2_ws
COPY --parents ./clover2*/ /clover2_ws/src/

FROM source AS builder
ARG CACHE_EPOCH
ARG ROS_DISTRO
ARG TARGETPLATFORM
ENV CCACHE_DIR="/root/.ccache"

COPY tooling/docker/* /clover2/
WORKDIR /clover2_ws

RUN --mount=type=cache,id=apt-ros-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=apt-ros-${TARGETPLATFORM},target=/var/lib/apt/lists,sharing=locked \
    apt-get update -y && \
    apt-get install -y \
        ccache \
        ros-${ROS_DISTRO}-ament-cmake-gtest

RUN --mount=type=cache,id=ccache-${CACHE_EPOCH}-${TARGETPLATFORM},target=${CCACHE_DIR} \
    ccache -M 5G

RUN --mount=type=cache,id=ccache-${CACHE_EPOCH}-${TARGETPLATFORM},target=${CCACHE_DIR} \
    --mount=type=cache,id=ros-build-${CACHE_EPOCH}-${TARGETPLATFORM},target=/clover2_ws/build \
    /clover2/build_ros.sh ${CCACHE_DIR} /opt/clover2 "--cmake-args -DBUILD_TESTING=1"

FROM builder AS test
ARG CACHE_EPOCH
ARG TARGETPLATFORM

RUN --mount=type=cache,id=ros-build-${CACHE_EPOCH}-${TARGETPLATFORM},target=/clover2_ws/build \
    /clover2/test_ros.sh

# from empty image
FROM scratch AS test-results
COPY --from=test /test-results /

FROM core AS clover2-ros

ARG ROS_DISTRO
ENV ROS_DISTRO=${ROS_DISTRO}

WORKDIR /clover2_ws
COPY --from=builder /opt/clover2 /opt/clover2

COPY docker/ros/entrypoint.sh  /

ENTRYPOINT ["/entrypoint.sh"]
CMD ["ros2", "launch", "clover2_bringup", "klever5.launch.xml"]
