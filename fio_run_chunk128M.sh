make clean

chunk_mb=128
chunk_blocks=$((chunk_mb * 1024 * 1024 / 4096))

SPDK_FTL_ZONE_EMU_BLOCKS=${chunk_blocks} make -j

for algo in sepbit
do
    for cc in cb
    do

        for dist in zipf:0.8
        do
            res_dir=/data/guntherxing/ftl_result_0807_2/${algo}_${cc}_${dist}
            if [ ! -d "$res_dir" ]; then
                mkdir -p "$res_dir"
                echo "Directory created: $res_dir"
            else
                echo "Directory already exists: $res_dir"
            fi

            for bs in 32k
            do
                for rw in rand
                do
                    for op in 20
                    do
                        echo "Running $algo $cc $dist $bs $rw OP$op"
                        LD_PRELOAD=/data/guntherxing/dev/spdk/build/fio/spdk_bdev \
                        /data/guntherxing/dev/fio/fio /data/guntherxing/dev/spdk/exp/job/${algo}/${cc}_${bs}_${rw}_op${op}_${dist}.job \
                        > ${res_dir}/${bs}_${rw}_op${op}.log \
                        2>&1
                    done
                done
            done

        done
    done
done