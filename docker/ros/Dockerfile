FROM ubuntu:noble

ARG ROS_DISTRO=jazzy

ENV DEBIAN_FRONTEND=noninteractive

# Setup locates
RUN echo 'Europe/Moscow' > /etc/timezone && \
    ln -s /usr/share/zoneinfo/Europe/Moscow /etc/localtime && \
    apt-get update && \
    apt-get install -q -y --no-install-recommends tzdata && \
    rm -rf /var/lib/apt/lists/*

# Install packages
RUN apt-get update && apt-get install -q -y --no-install-recommends \
    ca-certificates \
    curl \
    dirmngr \
    gnupg2 \
    && rm -rf /var/lib/apt/lists/*

# Setup ROS Apt sources
RUN curl -L -s -o /tmp/ros2-apt-source.deb https://github.com/ros-infrastructure/ros-apt-source/releases/download/1.1.0/ros2-apt-source_1.1.0.jammy_all.deb \
    && echo "1600cb8cc28258a39bffc1736a75bcbf52d1f2db371a4d020c1b187d2a5a083b /tmp/ros2-apt-source.deb" | sha256sum --strict --check \
    && apt-get update \
    && apt-get install /tmp/ros2-apt-source.deb \
    && rm -f /tmp/ros2-apt-source.deb \
    && rm -rf /var/lib/apt/lists/*

# Setup environment
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Install bootstrap tools
RUN apt-get update && apt-get install --no-install-recommends -y \
    build-essential \
    git \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    python3-rosdep \
    python3-vcstool \
    libasio-dev \
    && rm -rf /var/lib/apt/lists/*

# Bootstrap rosdep
RUN rosdep init && \
  rosdep update --rosdistro ${ROS_DISTRO}

# Install ros2 packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-${ROS_DISTRO}-ros-core=0.10.0-1* \
    ros-${ROS_DISTRO}-ros-base=0.10.0-1* \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app/clover2_ws/src
COPY . ./clover2

RUN . /opt/ros/${ROS_DISTRO}/setup.sh && colcon build --symlink-install