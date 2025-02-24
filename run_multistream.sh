#!/bin/bash

exp_prefix=(\
TRACE_ALGO_single_group_cb_OP_14.job
ALGO_sepbit44_cb_BS_64k_WP_rand_OP_14_DIS_zipf:0.8.job
ALGO_sepbit46_cb_BS_64k_WP_rand_OP_14_DIS_zipf:0.8.job
ALGO_mida44_cb_BS_64k_WP_rand_OP_14_DIS_zipf:0.8.job
ALGO_mida46_cb_BS_64k_WP_rand_OP_14_DIS_zipf:0.8.job
TRACE_ALGO_sepgc11_cb_OP_14.job
TRACE_ALGO_sepgc21_cb_OP_14.job
TRACE_ALGO_sepbit22_cb_OP_14.job
TRACE_ALGO_sepbit23_cb_OP_14.job
TRACE_ALGO_sepbit24_cb_OP_14.job
            )

for exp in "${exp_prefix[@]}"; do
    echo "Running $exp"
    sudo $HOME/spdk/fio/fio $HOME/spdk/exp-0219/jobs/$exp > $HOME/spdk/exp-0219/result2/$exp.out 2>&1
    # echo sudo /home/xgj/fio/fio /home/xgj/spdk/exp-0219/jobs/$exp
done
