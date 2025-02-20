#!/bin/bash


exp_prefix=(\
            "ALGO_single_group_greedy_BS_4k_WP_rand_OP_20_DIS_zipf:0.8" \
            "ALGO_sepbit_cb_BS_4k_WP_rand_OP_20_DIS_zipf:0.8" \
            )

for exp in "${exp_prefix[@]}"; do
    echo "Running $exp"
    sudo /home/xgj/fio/fio /home/xgj/spdk/exp-0219/jobs/$exp.job > /home/xgj/fio-out/$exp.out 2>&1
done
