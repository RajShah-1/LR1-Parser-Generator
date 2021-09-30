#!/bin/bash
bash buildModule1.sh
start_time=$(date +%s%N)
./build/Module_1/main.out < ./grammar_2/test1.txt
echo "Execution Time: $((($(date +%s%N) - $start_time)/1000000))ms"