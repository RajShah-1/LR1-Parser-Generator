#!/bin/bash
bash buildModule1.sh
start_time=$(date +%s%N)
./build/Module_1/main.out < ./grammar_2/inp.txt > ./grammar_2/out.txt
echo "_____________________________________________________________"
echo "Execution Time: $((($(date +%s%N) - $start_time)/1000000))ms"