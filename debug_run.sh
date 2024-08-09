# SPDK_FTL_ZONE_EMU_BLOCKS=32768 make -j64
# ./build/bin/spdk_tgt -m 0x3 -c /data/guntherxing/dev/spdk/exp/config/debug_ftl_op20.json

make clean

chunk_mb=1536
chunk_blocks=$((chunk_mb * 1024 * 1024 / 4096))

scl enable devtoolset-8 "SPDK_FTL_ZONE_EMU_BLOCKS=${chunk_blocks} make -j64"

# LD_PRELOAD=/data/guntherxing/dev/spdk/build/fio/spdk_bdev \
# /data/guntherxing/dev/fio/fio /data/guntherxing/dev/spdk/exp/job/template_trace.job \
