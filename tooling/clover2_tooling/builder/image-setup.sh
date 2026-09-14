#!/bin/bash
# Stage runner: sources stages/*.sh in order, tracks completion markers,
# supports selection (--stages/--skip), fresh runs (--fresh) and --list.
# Runs inside the target image; expects REGISTRY/CLOVER2_VERSION/CLOVER2_GIT_HASH
# in the environment (passed by 'clover2 builder build').
set -e

[[ "${DEBUG:-0}" == "1" ]] && set -x

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
CLOVER2_WS_DIR="/opt/clover2/ws"
STAGES_META_DIR="${STAGES_META_DIR:-/var/log/clover2/stages}"

# // options
ONLY_STAGES=""
SKIP_STAGES=""
FRESH=0
LIST=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --stages) ONLY_STAGES="$2"; shift 2 ;;
        --skip)   SKIP_STAGES="$2"; shift 2 ;;
        --fresh)  FRESH=1; shift ;;
        --list)   LIST=1; shift ;;
        *) log_error "Unknown option: $1" ;;
    esac
done

stage_selected() {
    local STAGE=$1

    if [ -n "$ONLY_STAGES" ]; then
        [[ ",$ONLY_STAGES," == *",$STAGE,"* ]] || return 1
    fi
    if [ -n "$SKIP_STAGES" ]; then
        [[ ",$SKIP_STAGES," == *",$STAGE,"* ]] && return 1
    fi

    return 0
}

if [ "$LIST" -eq 1 ]; then
    for stage in "$STAGES_DIR"/*.sh; do
        [ -f "$stage" ] && basename "$stage"
    done

    exit 0
fi

if [ "$FRESH" -eq 1 ]; then
    log_info "Fresh run: clearing markers in $STAGES_META_DIR"
    sudo rm -rf "$STAGES_META_DIR"
fi

sudo mkdir -p "$STAGES_META_DIR"
sudo chown "$USER" "$STAGES_META_DIR"

run_stage() {
    local STAGE_FILE=$1
    local STAGE=$(basename "$STAGE_FILE")
    local START END

    if [ -f "$STAGES_META_DIR/$STAGE.done" ]; then
        log_stage "Skip stage $STAGE (done)"
        return 0
    fi

    if ! stage_selected "$STAGE"; then
        log_stage "Skip stage $STAGE (not selected)"
        return 0
    fi

    log_stage "Process stage $STAGE"
    START=$(date +%s)
    source "$STAGE_FILE"
    END=$(date +%s)

    touch "$STAGES_META_DIR/$STAGE.done"
    log_stage "Stage $STAGE finished in $((END - START))s"

    cd /home/$USER

    sudo apt-get clean -y
    sudo apt-get autoclean -y
}

for stage in "$STAGES_DIR"/*.sh; do
    if [ -f "$stage" ]; then
        run_stage "$stage"
    fi
done

log_info "All stages processed"
