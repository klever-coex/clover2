ARG TARGETPLATFORM

FROM ubuntu:noble
ARG TARGETPLATFORM

ENV DEBIAN_FRONTEND=noninteractive \
    PIP_DISABLE_PIP_VERSION_CHECK=1 \
    PYTHONDONTWRITEBYTECODE=1

RUN --mount=type=cache,id=apt-builder-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=apt-builder-${TARGETPLATFORM},target=/var/lib/apt/lists,sharing=locked \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        curl \
        git \
        kpartx \
        make \
        python3-dev \
        python3-pip \
        qemu-system \
        qemu-system-aarch64 \
        qemu-utils \
        sudo \
        xz-utils && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /builder

COPY ./tooling/requirements.txt ./tooling-requirements.txt

RUN pip3 install --break-system-packages --no-cache-dir \
        -r ./tooling-requirements.txt \
    && rm ./tooling-requirements.txt

COPY ./docker/builder/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
