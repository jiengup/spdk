make clean

chunk_mb=1536
chunk_blocks=$((chunk_mb * 1024 * 1024 / 4096))

scl enable devtoolset-8 "SPDK_FTL_ZONE_EMU_BLOCKS=${chunk_blocks} make -j64"
