FROM ubuntu:24.04 AS clover2-px4-deps

ENV DEBIAN_FRONTEND=noninteractive
COPY tooling/docker/* /clover2/

# Install dependencies
RUN apt-get update && \
	apt-get install -y --no-install-recommends \
		build-essential \
		ccache \
		cmake \
		ninja-build \
		git \
		python3 \
		python3-dev \
		python3-pip \
		python3-setuptools \
		python3-wheel \
		libxml2-dev \
		libssl-dev \
		file \
		rsync \
		zip \
		unzip \
		wget \
		gnupg \
        lsb-release \
		libeigen3-dev \
        ca-certificates && \
	/clover2/cleanup-apt.sh

# Install gazebo
RUN wget https://packages.osrfoundation.org/gazebo.gpg -O /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg \
    && echo "deb [arch=amd64 signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" | tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null

RUN apt-get update -qq \
    && apt-get install -y \
        libgz-transport13-dev \
        libgz-msgs10-dev \
        libgz-tools2-dev \
        libgz-utils2-dev && \
	/clover2/cleanup-apt.sh


FROM clover2-px4-deps AS clover2-px4-dev
ARG PX4_VERSION

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /build

# Clone PX4
RUN git clone https://github.com/PX4/PX4-Autopilot.git
WORKDIR /build/PX4-Autopilot
RUN echo "Checkout PX4-Autopilot version: ${PX4_VERSION}" && \
    git switch --detach ${PX4_VERSION} && \
    git submodule update --init --recursive
RUN pip3 install -r ./Tools/setup/requirements.txt --no-cache-dir --break-system-packages

# Build
COPY docker/px4/patches/* /patches/
COPY tooling/scripts/* /clover2/
RUN /clover2/apply_patches.py /build/PX4-Autopilot /patches --verbose --recursive && \
    make px4_sitl_default


FROM clover2-px4-deps AS clover2-px4-sitl

ENV DEBIAN_FRONTEND=noninteractive
COPY tooling/docker/* /clover2/

WORKDIR /px4

COPY --from=clover2-px4-dev /build/PX4-Autopilot/build/px4_sitl_default/bin ./bin
COPY --from=clover2-px4-dev /build/PX4-Autopilot/build/px4_sitl_default/etc ./etc
COPY --from=clover2-px4-dev /build/PX4-Autopilot/ROMFS ./ROMFS

ENV PX4_GZ_STANDALONE=1
ENV PX4_GZ_MODEL_NAME=px4
ENV PX4_SYS_AUTOSTART=4001

ENTRYPOINT ["/px4/bin/px4", "ROMFS/px4fmu_common", "-d", "/px4/etc"]
