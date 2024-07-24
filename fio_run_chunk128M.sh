make clean
scl enable devtoolset-8 'SPDK_FTL_ZONE_EMU_BLOCKS=32768 make -j64'

for bs in 4k 64k 128k
do
    for rw in seq rand
    do
        for op in 20 40 60
        do
            echo "Running $bs $rw $op"
            LD_PRELOAD=/data/guntherxing/dev/spdk/build/fio/spdk_bdev \
            /data/guntherxing/dev/fio/fio /data/guntherxing/dev/spdk/exp/job/ftl_fio_${bs}_${rw}_${op}.job \
            > /data/guntherxing/dev/spdk/exp/result/ftl_fio_${bs}_${rw}_${op}.log \
            2>&1
        done
    done
done
