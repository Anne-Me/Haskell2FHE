#!/bin/bash

# Exit on any error
set -e

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <input_file_path> <output_file_name>"
  exit 1
fi

input_file="$1"
output_file="$2"
module="${3:-Example.Project}"

if [ ! -f "$input_file" ]; then
  echo "Error: Input file '$input_file' does not exist."
  exit 1
fi



#stack exec clash -- --verilog $input_file
stack exec --resolver lts-23.15 --package clash-ghc -- clash $input_file --verilog

echo "processing verilog/$module.topEntity/topEntity.v"

# generate temporary yosys synth script
cat <<EOF > synth.ys
read_verilog verilog/$module.topEntity/topEntity.v
flatten
synth
write_json $output_file
EOF

yosys synth.ys

rm synth.ys
rm -r verilog/
