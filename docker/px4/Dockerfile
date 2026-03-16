FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
	astyle \
	build-essential \
	ccache \
	cmake \
	cppcheck \
	file \
	g++ \
	gcc \
	gdb \
	git \
	lcov \
	libssl-dev \
	libxml2-dev \
	libxml2-utils \
	make \
	ninja-build \
	python3 \
	python3-dev \
	python3-pip \
	python3-setuptools \
	python3-wheel \
	rsync \
	shellcheck \
	unzip \
	zip \
    ca-certificates \
    gnupg \
    gosu \
    lsb-release \
    software-properties-common \
    sudo \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

RUN git clone https://github.com/PX4/PX4-Autopilot.git

RUN cd /build/PX4-Autopilot && \
    git switch --detach v1.16.1 && \
    git submodule update --init --recursive

WORKDIR /build/PX4-Autopilot

RUN pip install -r ./Tools/setup/requirements.txt --break-system-packages

RUN make px4_sitl_default


FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libeigen3-dev \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /px4

COPY --from=builder /build/PX4-Autopilot/build/px4_sitl_default/bin/* .
COPY --from=builder /build/PX4-Autopilot/build/px4_sitl_default/etc ./etc
COPY --from=builder /build/PX4-Autopilot/ROMFS ./ROMFS

ENTRYPOINT ["./px4", "ROMFS/px4fmu_common"]
