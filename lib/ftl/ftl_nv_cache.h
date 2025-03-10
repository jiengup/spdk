/*   SPDX-License-Identifier: BSD-3-Clause
 *   Copyright 2023 Solidigm All Rights Reserved
 *   Copyright (C) 2022 Intel Corporation.
 *   All rights reserved.
 */

#ifndef FTL_NV_CACHE_H
#define FTL_NV_CACHE_H

#include "spdk/stdinc.h"
#include "spdk/crc32.h"

#include "ftl_io.h"
#include "ftl_utils.h"
#include "ftl_internal.h"
#include "nvc/ftl_nvc_dev.h"

/*
 * FTL non volatile cache is divided into groups of blocks called chunks.
 * Size of each chunk is multiple of xfer size plus additional metadata.
 * For each block associated lba is stored in metadata. Cache space is
 * written chunk by chunk sequentially. When number of free chunks reaches
 * some threshold oldest chunks are moved from cache to backend storage to
 * create space for new user data.
 */

#define FTL_NVC_VERSION_0	0
#define FTL_NVC_VERSION_1	1
#define FTL_NVC_VERSION_2	2

#define FTL_NVC_VERSION_CURRENT FTL_NVC_VERSION_2

#define FTL_NV_CACHE_NUM_COMPACTORS 8

/*
 * Parameters controlling nv cache write throttling.
 *
 * The write throttle limit value is calculated as follows:
 * limit = compaction_average_bw * (1.0 + modifier)
 *
 * The modifier depends on the number of free chunks vs the configured threshold. Its value is
 * zero if the number of free chunks is at the threshold, negative if below and positive if above.
 */

/* Interval in milliseconds between write throttle updates. */
#define FTL_NV_CACHE_THROTTLE_INTERVAL_MS	20
/* Throttle modifier proportional gain */
#define FTL_NV_CACHE_THROTTLE_MODIFIER_KP	20
/* Min and max modifier values */
#define FTL_NV_CACHE_THROTTLE_MODIFIER_MIN	-0.8
#define FTL_NV_CACHE_THROTTLE_MODIFIER_MAX	0.5

#define FTL_MAX_OPEN_CHUNK_FACTOR 200
#define FTL_FREE_COMPACTION_THRESHOLD 10

struct ftl_nvcache_restore;
typedef void (*ftl_nv_cache_restore_fn)(struct ftl_nvcache_restore *, int, void *cb_arg);

enum ftl_chunk_state {
	FTL_CHUNK_STATE_FREE,
	FTL_CHUNK_STATE_OPEN,
	FTL_CHUNK_STATE_CLOSED,
	FTL_CHUNK_STATE_INACTIVE,
	FTL_CHUNK_STATE_MAX
};

struct ftl_nv_cache_chunk_md {
	/* Chunk metadata version */
	uint64_t version;

	/* Sequence id of writing */
	uint64_t seq_id;

	/* Sequence ID when chunk was closed */
	uint64_t close_seq_id;

	uint64_t timestamp;

	uint64_t close_timestamp;

	uint64_t last_invalid_timestamp;

	uint8_t  tag;

	/* Current lba to write */
	uint32_t write_pointer;

	/* Number of blocks written */
	uint32_t blocks_written;

	/* Number of skipped block (case when IO size is greater than blocks left in chunk) */
	uint32_t blocks_skipped;

	/* Next block to be compacted */
	uint32_t read_pointer;

	/* Number of compacted (both valid and invalid) blocks */
	uint32_t blocks_compacted;

	/* Chunk state */
	enum ftl_chunk_state state;

	/* CRC32 checksum of the associated P2L map when chunk is in closed state */
	uint32_t p2l_map_checksum;

	uint64_t valid_count;

	enum ftl_layout_region_type p2l_log_type;

	/* Reserved */
	uint8_t reserved[4007];
} __attribute__((packed));

SPDK_STATIC_ASSERT(sizeof(struct ftl_nv_cache_chunk_md) == FTL_BLOCK_SIZE,
		   "FTL NV Chunk metadata size is invalid");

struct ftl_nv_cache_chunk {
	struct ftl_nv_cache *nv_cache;

	struct ftl_nv_cache_chunk_md *md;

	/* Offset from start lba of the cache */
	uint64_t offset;

	uint64_t idx;
	uint64_t partition_idx;

	/* P2L map */
	struct ftl_p2l_map p2l_map;

	/* Metadata request */
	struct ftl_basic_rq metadata_rq;

	TAILQ_ENTRY(ftl_nv_cache_chunk) entry;

	/* This flag is used to indicate chunk is used in recovery */
	bool recovery;

	/* Compaction start time */
	uint64_t compaction_start_tsc;

	/* Compaction duration */
	uint64_t compaction_length_tsc;

	/* For writing metadata */
	struct ftl_md_io_entry_ctx md_persist_entry_ctx;

	/* P2L Log for IOs */
	struct ftl_p2l_log *p2l_log;
};

struct ftl_nv_cache_compactor {
	struct ftl_nv_cache *nv_cache;
	struct ftl_rq *rq;
	TAILQ_ENTRY(ftl_nv_cache_compactor) entry;
	struct spdk_bdev_io_wait_entry bdev_io_wait;
};

struct ftl_nv_cache {
	/* Flag indicating halt request */
	bool halt;

	/* NV cache device type */
	const struct ftl_nv_cache_device_type *nvc_type;

	/* Write buffer cache bdev */
	struct spdk_bdev_desc *bdev_desc;

	/* Persistent cache IO channel */
	struct spdk_io_channel *cache_ioch;

	/* Metadata pool */
	struct ftl_mempool *md_pool;

	/* P2L map memory pool */
	struct ftl_mempool *p2l_pool;

	/* Chunk md memory pool */
	struct ftl_mempool *chunk_md_pool;

	/* Chunk md memory pool for freeing chunks */
	struct ftl_mempool *free_chunk_md_pool;

	/* Block Metadata size */
	uint64_t md_size;

	/* NV cache metadata object handle */
	struct ftl_md *md;

	/* Number of blocks in chunk */
	uint64_t chunk_blocks;

	/* Number of blocks in tail md per chunk */
	uint64_t tail_md_chunk_blocks;

	/* Number of chunks */
	uint64_t chunk_count;

	/* Current processed chunk */
#define FTL_MAX_GROUP_NUM 10
	struct ftl_nv_cache_chunk *chunk_current[FTL_MAX_GROUP_NUM];

	uint64_t max_open_chunks;

	/* Free chunks list */
	TAILQ_HEAD(, ftl_nv_cache_chunk) chunk_free_list;
	uint64_t chunk_free_count;

	/* Open chunks list */
	TAILQ_HEAD(, ftl_nv_cache_chunk) chunk_open_list;
	uint64_t chunk_open_count;
	uint64_t chunk_aopen_count;

	/* Full chunks list */
	TAILQ_HEAD(, ftl_nv_cache_chunk) chunk_full_list;
	uint64_t chunk_full_count;

	/* Chunks being compacted */
	TAILQ_HEAD(, ftl_nv_cache_chunk) chunk_comp_list;
	uint64_t chunk_comp_count;

	/* Chunks being freed */
	TAILQ_HEAD(, ftl_nv_cache_chunk) needs_free_persist_list;
	uint64_t chunk_free_persist_count;

	/* Chunks which are inactive */
	TAILQ_HEAD(, ftl_nv_cache_chunk) chunk_inactive_list;
	uint64_t chunk_inactive_count;

	TAILQ_HEAD(, ftl_nv_cache_compactor) compactor_list;
	uint64_t compaction_active_count;
	uint64_t chunk_compaction_threshold;

	struct ftl_nv_cache_chunk *chunks;

	uint64_t last_seq_id;

	uint64_t chunk_free_target;

	uint64_t chunk_free_compaction_target;

	/* Simple moving average of recent compaction velocity values */
	double compaction_sma;

	uint8_t traffic_group_num;
	uint8_t partition_num;
#define FTL_NV_CACHE_MAX_PARTITION_NUM 4
	uint64_t partition_chunk_count[FTL_NV_CACHE_MAX_PARTITION_NUM];

#define FTL_NV_CACHE_COMPACTION_SMA_N (FTL_NV_CACHE_NUM_COMPACTORS * 2)
	/* Circular buffer holding values for calculating compaction SMA */
	struct compaction_bw_stats {
		double buf[FTL_NV_CACHE_COMPACTION_SMA_N];
		ptrdiff_t first;
		size_t count;
		double sum;
	} compaction_recent_bw;

	struct {
		uint64_t interval_tsc;
		uint64_t start_tsc;
		uint64_t blocks_submitted;
		uint64_t blocks_submitted_limit;
	} throttle;

	uint64_t *lba_timestamp_table;

#define FTL_NV_CACHE_SEPBIT_PERIOD 16
	struct {
		uint64_t threshold;
		uint64_t threshold_total;
		uint8_t  count;
	} sepbit_info;
};

typedef void (*nvc_scrub_cb)(struct spdk_ftl_dev *dev, void *cb_ctx, int status);

void ftl_nv_cache_scrub(struct spdk_ftl_dev *dev, nvc_scrub_cb cb, void *cb_ctx);

int ftl_nv_cache_init(struct spdk_ftl_dev *dev);
void ftl_nv_cache_deinit(struct spdk_ftl_dev *dev);
bool ftl_nv_cache_write(struct ftl_io *io);
void ftl_nv_cache_write_complete(struct ftl_io *io, bool success);
void ftl_nv_cache_fill_md(struct ftl_io *io);
int ftl_nv_cache_read(struct ftl_io *io, ftl_addr addr, uint32_t num_blocks,
		      spdk_bdev_io_completion_cb cb, void *cb_arg);
bool ftl_nv_cache_throttle(struct spdk_ftl_dev *dev);
void ftl_nv_cache_process(struct spdk_ftl_dev *dev);

void ftl_chunk_map_set_lba(struct ftl_nv_cache_chunk *chunk,
			   uint64_t offset, uint64_t lba);
uint64_t ftl_chunk_map_get_lba(struct ftl_nv_cache_chunk *chunk, uint64_t offset);

void ftl_nv_cache_set_addr(struct spdk_ftl_dev *dev, uint64_t lba, ftl_addr addr);

void ftl_nv_cache_chunk_set_addr(struct ftl_nv_cache_chunk *chunk, uint64_t lba, ftl_addr addr);

void ftl_nv_cache_clear_addr(struct spdk_ftl_dev *dev, ftl_addr addr);

int ftl_nv_cache_save_state(struct ftl_nv_cache *nv_cache);

int ftl_nv_cache_load_state(struct ftl_nv_cache *nv_cache);

void ftl_nv_cache_halt(struct ftl_nv_cache *nv_cache);

int ftl_nv_cache_chunks_busy(struct ftl_nv_cache *nv_cache);

static inline void
ftl_nv_cache_resume(struct ftl_nv_cache *nv_cache)
{
	nv_cache->halt = false;
}

bool ftl_nv_cache_is_halted(struct ftl_nv_cache *nv_cache);

size_t ftl_nv_cache_chunk_tail_md_num_blocks(const struct ftl_nv_cache *nv_cache);

uint64_t chunk_tail_md_offset(struct ftl_nv_cache *nv_cache);
/**
 * @brief Iterates over NV caches chunks and returns the max open and closed sequence id
 *
 * @param nv_cache FLT NV cache
 * @param[out] open_seq_id Max detected open sequence id
 * @param[out] close_seq_id Max detected close sequence id
 */
void ftl_nv_cache_get_max_seq_id(struct ftl_nv_cache *nv_cache, uint64_t *open_seq_id,
				 uint64_t *close_seq_id);

void ftl_mngt_nv_cache_restore_chunk_state(struct spdk_ftl_dev *dev, struct ftl_mngt_process *mngt);

void ftl_mngt_nv_cache_recover_open_chunk(struct spdk_ftl_dev *dev, struct ftl_mngt_process *mngt);

typedef int (*ftl_chunk_md_cb)(struct ftl_nv_cache_chunk *chunk, void *cntx);

void ftl_mngt_nv_cache_restore_l2p(struct spdk_ftl_dev *dev, struct ftl_mngt_process *mngt,
				   ftl_chunk_md_cb cb, void *cb_ctx);

struct ftl_nv_cache_chunk *ftl_nv_cache_get_chunk_from_addr(struct spdk_ftl_dev *dev,
		ftl_addr addr);

uint64_t ftl_nv_cache_acquire_trim_seq_id(struct ftl_nv_cache *nv_cache);

void ftl_nv_cache_chunk_md_initialize(struct ftl_nv_cache_chunk_md *md);

#endif  /* FTL_NV_CACHE_H */
