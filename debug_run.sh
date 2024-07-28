# SPDK_FTL_ZONE_EMU_BLOCKS=32768 make -j64

scl enable devtoolset-8 'SPDK_FTL_ZONE_EMU_BLOCKS=32768 make -j64'
./build/bin/spdk_tgt -m 0x3 -c /data/guntherxing/dev/spdk/exp/config/debug_ftl_op20.json
