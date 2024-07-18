SPDK_FTL_ZONE_EMU_BLOCKS=32768 make -j64
./build/bin/spdk_tgt -m 0x3 -c test/ftl/config/ftl.json
