#!/bin/bash

git config --global --add safe.directory /builder

# Re-sync the venv against the mounted tooling, so the container always runs
# the repo's current code (the baked venv is just the offline fallback).
# Subshell: don't leak the cd into the user's command.
if [ -d /builder/tooling ]; then
    (cd /builder/tooling && uv sync --frozen --no-dev)
fi

exec "$@"
