#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <source_directory> <destination_directory>"
    exit 1
fi

SOURCE_DIR="$1"
DEST_DIR="$2"

if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Source directory '$SOURCE_DIR' does not exist."
    exit 1
fi

mkdir -p "$DEST_DIR"

echo "Copying from '$SOURCE_DIR' to '$DEST_DIR' (excluding .gitignore files/dirs)..."
rsync -av --exclude-from='.gitignore' "$SOURCE_DIR"/ "$DEST_DIR"/
