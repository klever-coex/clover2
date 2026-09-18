#!/bin/bash
set -eo pipefail

source /opt/ros/jazzy/setup.bash
source /opt/clover2/setup.bash


colcon test --build-base /clover2_ws/build \
            --install-base /opt/clover2 \
            --merge-install \
            --ctest-args -R "test_*"

mkdir -p /test-results
find /clover2_ws/build -path '*/test_results/*.xml' -exec cp --parents {} /test-results/ \;

if ! colcon test-result --all; then
  touch /test-results/FAILED
fi
