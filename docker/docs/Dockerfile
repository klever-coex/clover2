ARG BUILDPLATFORM
ARG TARGETPLATFORM

FROM --platform=${BUILDPLATFORM} python:3.11-slim AS doxygen
ARG TARGETPLATFORM

RUN --mount=type=cache,id=apt-docs-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=apt-docs-${TARGETPLATFORM},target=/var/lib/apt/lists,sharing=locked \
    apt-get update && \
    apt-get install -y doxygen graphviz make && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY Doxyfile /app
COPY --parents ./**/*.hpp /app

RUN mkdir -p /app/docs/build/doxygen && doxygen

FROM --platform=${BUILDPLATFORM} python:3.11-slim AS builder
ARG TARGETPLATFORM

ARG CLOVER2_VERSION
ARG DEBIAN_FRONTEND=noninteractive

RUN --mount=type=cache,id=apt-docs-${TARGETPLATFORM},target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=apt-docs-${TARGETPLATFORM},target=/var/lib/apt/lists,sharing=locked \
    apt-get update && \
    apt-get install -y make && \
    rm -rf /var/lib/apt/lists/*

ENV CLOVER2_VERSION=${CLOVER2_VERSION}

WORKDIR /app
COPY docs/requirements.txt /app/requirements.txt

RUN --mount=type=cache,id=pip-docs,target=/root/.cache/pip \
    pip3 install --break-system-packages -r requirements.txt

COPY docs /app
COPY --from=doxygen /app/docs/build/doxygen /app/build/doxygen

RUN make check-images
RUN make html

FROM nginx:1.29.3-alpine-slim

COPY --from=docs-html /app/build/html /usr/share/nginx/html

COPY docker/docs/nginx.conf /etc/nginx/nginx.conf

CMD ["nginx", "-g", "daemon off;"]
