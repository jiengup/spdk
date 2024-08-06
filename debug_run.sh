# SPDK_FTL_ZONE_EMU_BLOCKS=32768 make -j64
# ./build/bin/spdk_tgt -m 0x3 -c /data/guntherxing/dev/spdk/exp/config/debug_ftl_op20.json

scl enable devtoolset-8 'SPDK_FTL_ZONE_EMU_BLOCKS=16384 make -j64'
LD_PRELOAD=/data/guntherxing/dev/spdk/build/fio/spdk_bdev \
/data/guntherxing/dev/fio/fio /data/guntherxing/dev/spdk/exp/job/template_trace.job \
