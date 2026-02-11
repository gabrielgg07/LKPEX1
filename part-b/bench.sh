#!/bin/bash
# bench.sh - Collect benchmark data across different dataset sizes
#
# Usage: sudo ./bench.sh > bench_data.txt
#
# Output format (one header line + one row per size):
# N  list_ins  hash_ins  rb_ins  xa_ins  list_lkp  hash_lkp  rb_lkp  xa_lkp

set -euo pipefail

echo "# N  list_ins  hash_ins  rb_ins  xa_ins  list_lkp  hash_lkp  rb_lkp  xa_lkp"

for n in 100 1000 5000 10000 50000; do
    sudo insmod lkp_ds.ko int_str="1" bench_size=$n
    vals=$(awk -F':' '/:/{print $2}' /proc/lkp_ds_bench | awk '{print $1}' | tr '\n' ' ')
    echo "$n $vals"
    cat /proc/lkp_ds_bench
    sudo rmmod lkp_ds
    sleep 1
done
