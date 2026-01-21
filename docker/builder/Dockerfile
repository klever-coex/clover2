FROM ubuntu:noble

# Install dependencies
RUN apt-get update && \
    apt-get install -y \
        build-essential \
        curl \
        git \
        kpartx \
        make \
        python3-dev \
        python3-venv \
        qemu-system \
        qemu-system-aarch64 \
        qemu-utils \
        sudo \
        xz-utils && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /builder
COPY ./docker/builder/requirements.txt ./requirements.txt

RUN /bin/bash -c "python3 -m venv /docker_venv && \
    . /docker_venv/bin/activate && \
    pip3 install -r ./requirements.txt && \
    rm ./requirements.txt"

COPY ./docker/builder/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
