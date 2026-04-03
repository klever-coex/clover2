FROM debian:bookworm-slim AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV CCACHE_DIR=/root/.ccache
COPY tooling/docker/* /clover2/

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
		ca-certificates && \
	/clover2/cleanup-apt.sh

WORKDIR /build

RUN git clone https://github.com/PX4/PX4-Autopilot.git

WORKDIR /build/PX4-Autopilot

RUN git switch --detach v1.16.1 && \
    git submodule update --init --recursive

RUN pip3 install -r ./Tools/setup/requirements.txt --no-cache-dir --break-system-packages

RUN make px4_sitl_default

FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
	apt-get install -y --no-install-recommends \
		libstdc++6 \
		python3 \
		libeigen3-dev && \
	/clover2/cleanup-apt.sh

WORKDIR /px4

COPY --from=builder /build/PX4-Autopilot/build/px4_sitl_default/bin/px4 .
COPY --from=builder /build/PX4-Autopilot/build/px4_sitl_default/etc ./etc
COPY --from=builder /build/PX4-Autopilot/ROMFS ./ROMFS

ENTRYPOINT ["./px4", "ROMFS/px4fmu_common"]
