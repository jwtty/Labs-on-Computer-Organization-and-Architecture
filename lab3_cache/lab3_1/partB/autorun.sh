#!/bin/sh
echo "RISCV simulator to run dhrystone:"
cd riscv_sim 
make clean > /dev/null 2>&1
make > /dev/null 2>&1
./RV_SIM ./dry2reg
make clean > /dev/null 2>&1

cd ..
cd cache_sim
make clean > /dev/null 2>&1
make > /dev/null 2>&1
./sim ../riscv_sim/t.trace
make clean > /dev/null 2>&1

cd ..
cd riscv_sim
rm -f t.trace
