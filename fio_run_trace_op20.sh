make clean

chunk_mb=1536
chunk_blocks=$((chunk_mb * 1024 * 1024 / 4096))

scl enable devtoolset-8 "SPDK_FTL_ZONE_EMU_BLOCKS=${chunk_blocks} make -j64"

for algo in sepgc
do
    for cc in cb
    do

        res_dir=/data/guntherxing/ftl_result_0811/trace_${algo}_${cc}
        if [ ! -d "$res_dir" ]; then
            mkdir -p "$res_dir"
            echo "Directory created: $res_dir"
        else
            echo "Directory already exists: $res_dir"
        fi

        echo "Running $algo $cc OP20"
        LD_PRELOAD=/data/guntherxing/dev/spdk/build/fio/spdk_bdev \
        /data/guntherxing/dev/fio/fio /data/guntherxing/dev/spdk/exp/job/${algo}/trace_${cc}_op20.job \
        > ${res_dir}/op${op}.log \
        2>&1
    done
done