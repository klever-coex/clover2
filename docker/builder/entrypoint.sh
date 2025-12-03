#!/bin/bash

source /docker_venv/bin/activate
git config --global --add safe.directory /builder

exec "$@"
