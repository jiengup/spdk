#!/bin/bash

# Check if the input file is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <task_file>"
    exit 1
fi

input_file=$1

if [ ! -f "$input_file" ]; then
    echo "File not found: $task_file"
    exit 1
fi

while IFS= read -r line; do
    # Split the line into two strings
    param1=$(echo "$line" | cut -d' ' -f1)
    param2=$(echo "$line" | cut -d' ' -f2)
    echo "Running: $param1"
    sudo $HOME/spdk/fio/fio "$param1" > "$param2" 2>&1
    # echo "$HOME/spdk/fio/fio $param1 > $param2"
done < "$input_file"