make clean

chunk_mb=128
chunk_blocks=$((chunk_mb * 1024 * 1024 / 4096))

scl enable devtoolset-8 "SPDK_FTL_ZONE_EMU_BLOCKS=${chunk_blocks} make -j64"

algo=random_group
res_dir=/data/guntherxing/dev/spdk/exp/result/${algo}

if [ ! -d "$res_dir" ]; then
    mkdir -p "$res_dir"
    echo "Directory created: $res_dir"
else
    echo "Directory already exists: $res_dir"
fi

for bs in 4k 16k 128k
do
    for rw in rand
    do
        for op in 20 40 60
        do
            echo "Running $bs $rw $op"
            LD_PRELOAD=/data/guntherxing/dev/spdk/build/fio/spdk_bdev \
            /data/guntherxing/dev/fio/fio /data/guntherxing/dev/spdk/exp/job/${algo}/${bs}_${rw}_op${op}.job \
            > ${res_dir}/${bs}_${rw}_op${op}.log \
            2>&1
        done
    done
done
