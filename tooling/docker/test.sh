#!/bin/bash
set -eo pipefail

source /opt/ros/jazzy/setup.bash
source /opt/clover2/setup.bash

# The builder stage builds the workspace with BUILD_TESTING=1 (see the
# Dockerfile), so the test executables already exist in /clover2_ws/build.
# `colcon test` only runs ctest - it does not reconfigure or rebuild.
# -R "test_*" runs only the gtest suites (ament gtest targets are named
# test_<package>), skipping the lint tests.
colcon test --build-base /clover2_ws/build \
            --install-base /opt/clover2 \
            --merge-install \
            --ctest-args -R "test_*"

# Export the gtest XMLs (kept even on failure) to /test-results; the
# test-results stage copies this into the bake's local output.
mkdir -p /test-results
find /clover2_ws/build -path '*/test_results/*.xml' -exec cp --parents {} /test-results/ \;

# Gate: the workflow fails the job when this marker is present, while the
# XMLs above are still exported and published by test-report.yml.
if ! colcon test-result --all; then
  touch /test-results/FAILED
fi
