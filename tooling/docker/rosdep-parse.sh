#!/bin/bash
set -eo pipefail

function rosdep_parse() {
    local src_path=$1
    local ros_distro=$2
    local rosdep_keys_args=$3

    rosdep keys $rosdep_keys_args --ignore-src --from-paths "$src_path" |
        xargs rosdep resolve --rosdistro "$ros_distro" |
        grep -v '^#' |
        sed 's/ \+/\n/g'
}

rosdep_parse "$@"
