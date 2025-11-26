FROM debian:bookworm-slim

# Install dependencies
RUN apt-get update && \
    apt-get install -y \
        curl \
        git \
        python3-dev \
        python3-venv && \
        qemu-system \
        qemu-system-aarch64 \
        qemu-utils \
    rm -rf /var/lib/apt/lists/*

# Install task-go
RUN curl -1sLf 'https://dl.cloudsmith.io/public/task/task/setup.deb.sh' | bash
RUN apt-get update && \
    apt-get install -y \
        task && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /builder
COPY ./docker/builder/requirements.txt ./requirements.txt

RUN /bin/bash -c "python3 -m venv /docker_venv && \
    . /docker_venv/bin/activate && \
    pip3 install -r ./requirements.txt && \
    rm ./requirements.txt"
