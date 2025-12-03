#!/bin/bash
set -ex # Exit on any error

ROS_DISTRO=jazzy
USER=pi

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

log_stage() {
    echo -e "${BLUE}[STAGE]${NC} $1"
}

BUILDER_DIR=$(dirname "$(readlink -f "$0")")
REPO_DIR=$(readlink -m "$BUILDER_DIR/../..")
ASSETS_DIR="$BUILDER_DIR/assets"
STAGES_DIR="$BUILDER_DIR/stages"
STAGES_LOG_DIR=".stages_meta"

run_stage() {
    local STAGE_FILE=$1
    local STAGE=$(basename "$STAGE_FILE")

    if [ -f "$STAGES_LOG_DIR/$STAGE.done" ]; then
        log_stage "Skip stage $STAGE"
    else
        log_stage "Process stage $STAGE"
        source $STAGE_FILE
    fi

    log_info "Clean apt cache"
    sudo apt-get autoclean
    sudo apt-get clean

    cd /home/$USER
}

mkdir -p $STAGES_LOG_DIR

for stage in "$STAGES_DIR"/*; do
    if [ -f "$stage" ]; then
        run_stage "$stage"
    fi
done
