ARG TARGETPLATFORM

FROM ubuntu:noble
ARG TARGETPLATFORM

ENV DEBIAN_FRONTEND=noninteractive \
    UV_LINK_MODE=copy \
    UV_PROJECT_ENVIRONMENT=/opt/clover2-tooling-venv \
    UV_FROZEN=1 \
    PATH="/opt/clover2-tooling-venv/bin:${PATH}"

COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /usr/local/bin/

RUN --mount=type=cache,id=apt-builder-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=apt-builder-${TARGETPLATFORM},target=/var/lib/apt/lists,sharing=locked \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        curl \
        git \
        kpartx \
        make \
        python3 \
        qemu-system \
        qemu-system-aarch64 \
        qemu-utils \
        sudo \
        xz-utils && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /builder

COPY ./tooling /tooling
RUN cd /tooling && uv sync --frozen --no-dev

COPY ./docker/tooling/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
