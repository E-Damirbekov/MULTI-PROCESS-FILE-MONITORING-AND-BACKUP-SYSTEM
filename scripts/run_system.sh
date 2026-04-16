#!/bin/bash

# Move to the project root (where Makefile and main.c are)
# This ensures paths like 'src' and 'logs' work correctly
cd "$(dirname "$0")/.."

# Variables
SOURCE_DIR="src"
LOG_FILE="logs/report.txt"
BINARY="./monitor"

echo "=== Starting File Monitoring System ==="

# Step 1: Compile the project
make
if [ $? -ne 0 ]; then
    echo "ERROR: Compilation failed. Exiting."
    exit 1
fi
echo "Compilation successful."

# Step 2: Ensure required directories exist
mkdir -p logs alerts backup

# Step 3: Check that source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
    echo "ERROR: Source directory '$SOURCE_DIR' not found."
    exit 1
fi

# Step 4: Run the monitor
# We don't redirect to LOG_FILE because the C code handles logging via pipe
$BINARY $SOURCE_DIR

# Step 5: Check if monitor ran successfully
if [ $? -eq 0 ]; then
    echo "SUCCESS: Monitoring complete. Check logs/ and alerts/ for results."
else
    echo "ERROR: Monitor encountered a problem."
    exit 1
fi