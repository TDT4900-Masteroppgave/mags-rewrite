#!/bin/bash

# Define paths
DATA_DIR="tests/data"
FILE_NAME="email-Eu-core.txt"
URL="https://snap.stanford.edu/data/email-Eu-core.txt.gz"

# 1. Create directory if it doesn't exist
if [ ! -d "$DATA_DIR" ]; then
    echo "[INFO] Creating directory: $DATA_DIR"
    mkdir -p "$DATA_DIR"
fi

# 2. Check if file already exists
if [ -f "$DATA_DIR/$FILE_NAME" ]; then
    echo "[INFO] Dataset already exists at $DATA_DIR/$FILE_NAME. Skipping download."
    exit 0
fi

# 3. Download and Unzip
echo "[INFO] Downloading dataset from SNAP..."

# Use curl to download and gunzip in one go
# -L follows redirects, -o - outputs to stdout
if curl -L "$URL" | gunzip > "$DATA_DIR/$FILE_NAME"; then
    echo "[SUCCESS] Downloaded and extracted to $DATA_DIR/$FILE_NAME"
else
    echo "[ERROR] Failed to download dataset."
    # Clean up empty file if failed
    rm -f "$DATA_DIR/$FILE_NAME"
    exit 1
fi