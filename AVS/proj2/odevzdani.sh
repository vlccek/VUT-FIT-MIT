#!/bin/bash

# Define the output zip file
export login="xvlkja07"
output_zip="$login.zip"

# Define the list of files and directories to include in the zip
files=(
    "parallel_builder/loop_mesh_builder.h"
    "parallel_builder/loop_mesh_builder.cpp"
    "parallel_builder/tree_mesh_builder.h"
    "parallel_builder/tree_mesh_builder.cpp"
    "PMC-$login.txt"
    "4_1.txt"
    "4_2.txt"
    "4_3.txt"
    "3_4.txt"
    "input_scaling_strong.png"
    "input_scaling_weak.png"
    "grid_scaling.png"
)

# Delete the old ZIP archive if it exists
if [ -f "$output_zip" ]; then
    rm "$output_zip"
fi

# Create the ZIP archive with folder structure
for file in "${files[@]}"; do
    if [ -f "$file" ] || [ -d "$file" ]; then
        zip -r "$output_zip" "$file"
    else
        echo "Warning: File or directory $file not found!"
    fi
done

echo "Archive $output_zip created successfully."
