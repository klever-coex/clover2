#!/bin/bash
set -e # Exit on any error

ROS_DISTRO=jazzy

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

stage() {
    echo -e "${BLUE}[STAGE]${NC} $1"
}

BUILDER_DIR=$(dirname "$(readlink -f "$0")")
REPO_DIR=$(readlink -m "$BUILDER_DIR/../..")
ASSETS_DIR="$BUILDER_DIR/assets"
STAGES_DIR="$BUILDER_DIR/stages"

run_stage() {
    local STAGE_FILE=$1
    local STAGE=$(basename "$STAGE_FILE")
    stage "Process stage $STAGE"

    source $STAGE_FILE
}

for stage in "$STAGES_DIR"/*; do
    if [ -f "$stage" ]; then
        run_stage "$stage"
    fi
done
