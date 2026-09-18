#!/bin/bash
set -eo pipefail

function build() {
    local ccache_dir=$1
    local install_base=$2
    local colcon_build_args=$3

    export CCACHE_DIR="$ccache_dir"

    source /opt/ros/jazzy/setup.bash

    colcon build --install-base "$install_base" \
                 --merge-install \
                 --mixin release compile-commands ccache \
                 $colcon_build_args
}

build "$@"
