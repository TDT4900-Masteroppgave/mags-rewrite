#!/bin/bash

# 1. Setup absolute paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# 2. Define category folders
SMALL_DIR="$PROJECT_ROOT/data/small"
LARGE_DIR="$PROJECT_ROOT/data/large"

echo "[INFO] Project Root: $PROJECT_ROOT"
mkdir -p "$SMALL_DIR" "$LARGE_DIR"

# 3. Function to download and unzip
download_dataset() {
    local url=$1
    local output_name=$2
    local target_dir=$3

    if [ -f "$target_dir/$output_name" ]; then
        echo "[SKIP] $output_name already exists in $(basename "$target_dir")"
    else
        echo "[DOWNLOADING] $output_name..."
        # -f ensures we fail on 404 instead of saving an error page as a text file
        if curl -L -f "$url" | gunzip > "$target_dir/$output_name"; then
             echo "[SUCCESS] Saved to $target_dir/$output_name"
        else
             echo "[ERROR] Failed to download $output_name"
             rm -f "$target_dir/$output_name"
        fi
    fi
}

echo "========================================================"
echo "Downloading Small Graphs"
echo "========================================================"
download_dataset "https://snap.stanford.edu/data/as-caida20071105.txt.gz" "as-caida20071105.txt" "$SMALL_DIR"
download_dataset "https://snap.stanford.edu/data/email-Enron.txt.gz" "Email-Enron.txt" "$SMALL_DIR"
download_dataset "https://snap.stanford.edu/data/loc-brightkite_edges.txt.gz" "Brightkite_edges.txt" "$SMALL_DIR"
download_dataset "https://snap.stanford.edu/data/email-EuAll.txt.gz" "Email-EuAll.txt" "$SMALL_DIR"
download_dataset "https://snap.stanford.edu/data/soc-Slashdot0902.txt.gz" "Slashdot0902.txt" "$SMALL_DIR"
download_dataset "https://snap.stanford.edu/data/bigdata/communities/com-dblp.ungraph.txt.gz" "com-dblp.ungraph.txt" "$SMALL_DIR"

echo "========================================================"
echo "Downloading Large Graphs"
echo "========================================================"
download_dataset "https://snap.stanford.edu/data/amazon0601.txt.gz" "amazon0601.txt" "$LARGE_DIR"
download_dataset "https://snap.stanford.edu/data/bigdata/communities/com-youtube.ungraph.txt.gz" "com-youtube.ungraph.txt" "$LARGE_DIR"
download_dataset "https://snap.stanford.edu/data/as-skitter.txt.gz" "as-skitter.txt" "$LARGE_DIR"
download_dataset "https://snap.stanford.edu/data/bigdata/communities/com-lj.ungraph.txt.gz" "com-lj.ungraph.txt" "$LARGE_DIR"

echo "========================================================"
echo "Download Complete."