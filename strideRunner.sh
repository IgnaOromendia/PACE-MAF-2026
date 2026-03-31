#!/bin/sh

set -eu

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <instances-name>" >&2
    exit 1
fi

list_name="$1"
list_path="./input/lists/${list_name}.lst"

if [ ! -f "$list_path" ]; then
    echo "List file not found: $list_path" >&2
    exit 1
fi

make clean all
rm -rf stride-logs
./stride download -i "$list_path"
./stride run -s ./paceSolver -i "$list_path"
python3 process_latest_summary.py