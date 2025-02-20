#!/bin/bash

exp_prefix=(\
ALGO_sepbit22_cb_BS_32k_WP_rand_OP_15_DIS_zipf:1.1.job
ALGO_sepbit24_cb_BS_32k_WP_rand_OP_15_DIS_zipf:1.1.job
ALGO_sepbit44_cb_BS_32k_WP_rand_OP_15_DIS_zipf:1.1.job
ALGO_sepbit46_cb_BS_32k_WP_rand_OP_15_DIS_zipf:1.1.job
ALGO_mida22_cb_BS_32k_WP_rand_OP_15_DIS_zipf:0.8.job
ALGO_mida24_cb_BS_32k_WP_rand_OP_15_DIS_zipf:0.8.job
ALGO_mida44_cb_BS_32k_WP_rand_OP_15_DIS_zipf:0.8.job
ALGO_mida46_cb_BS_32k_WP_rand_OP_15_DIS_zipf:0.8.job
ALGO_sepbit22_cb_BS_32k_WP_rand_OP_15_DIS_zipf:0.8.job
ALGO_sepbit24_cb_BS_32k_WP_rand_OP_15_DIS_zipf:0.8.job
ALGO_sepbit44_cb_BS_32k_WP_rand_OP_15_DIS_zipf:0.8.job
ALGO_sepbit46_cb_BS_32k_WP_rand_OP_15_DIS_zipf:0.8.job
            )

for exp in "${exp_prefix[@]}"; do
    echo "Running $exp"
    sudo /home/xgj/fio/fio /home/xgj/spdk/exp-0219/jobs/$exp > /home/xgj/fio-out/$exp.out 2>&1
    # echo sudo /home/xgj/fio/fio /home/xgj/spdk/exp-0219/jobs/$exp
done
