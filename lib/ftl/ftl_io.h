/*   SPDX-License-Identifier: BSD-3-Clause
 *   Copyright (C) 2018 Intel Corporation.
 *   All rights reserved.
 */

#ifndef FTL_IO_H
#define FTL_IO_H

#include "spdk/stdinc.h"
#include "spdk/nvme.h"
#include "spdk/ftl.h"
#include "spdk/bdev.h"
#include "spdk/util.h"

#include "ftl_internal.h"
#include "ftl_trace.h"
#include "ftl_l2p.h"
#include "utils/ftl_md.h"

struct spdk_ftl_dev;
struct ftl_band;
struct ftl_io;

typedef void (*ftl_io_fn)(struct ftl_io *, void *, int);

/* IO flags */
enum ftl_io_flags {
	/* Indicates whether IO is already initialized */
	FTL_IO_INITIALIZED	= (1 << 0),
	/* Indicated whether the user IO pinned the L2P pages containing LBAs */
	FTL_IO_PINNED		= (1 << 1),
};

enum ftl_io_type {
	FTL_IO_READ,
	FTL_IO_WRITE,
	FTL_IO_TRIM,
};

#define FTL_IO_MAX_IOVEC 4

struct ftl_io_channel {
	/*  Device */
	struct spdk_ftl_dev		*dev;
	/*  Entry of IO channels queue/list */
	TAILQ_ENTRY(ftl_io_channel)	entry;
	/*  IO map pool */
	struct ftl_mempool		*map_pool;
	/*  Poller used for completing user requests and retrying IO */
	struct spdk_poller		*poller;
	/*  Submission queue */
	struct spdk_ring		*sq;
	/*  Completion queue */
	struct spdk_ring		*cq;
};

/* General IO descriptor for user requests */
struct ftl_io {
	/* Device */
	struct spdk_ftl_dev		*dev;

	/* IO channel */
	struct spdk_io_channel		*ioch;

	/* LBA address */
	uint64_t			lba;
	
	/* User io tag using for grouping */
	uint32_t			tag;
	uint64_t 			timestamp;

	/* First address of write when sent to cache device */
	ftl_addr			addr;
	ftl_addr 			chunk_offset;

	/* Number of processed blocks */
	size_t				pos;

	/* Number of blocks */
	size_t				num_blocks;

	/* IO vector pointer */
	struct iovec			*iov;

	/* Metadata */
	void				*md;

	/* Number of IO vectors */
	size_t				iov_cnt;

	/* Position within the io vector array */
	size_t				iov_pos;

	/* Offset within the iovec (in blocks) */
	size_t				iov_off;

	/* Band this IO is being written to */
	struct ftl_band			*band;

	/* Request status */
	int				status;

	/* Number of split requests */
	size_t				req_cnt;

	/* Callback's context */
	void				*cb_ctx;

	/* User callback function */
	spdk_ftl_fn			user_fn;

	/* Flags */
	int				flags;

	/* IO type */
	enum ftl_io_type		type;

	/* Done flag */
	bool				done;

	/* Trace group id */
	uint64_t			trace;

	/* Used by retry and write completion queues */
	TAILQ_ENTRY(ftl_io)		queue_entry;

	/* Reference to the chunk within NV cache */
	struct ftl_nv_cache_chunk	*nv_cache_chunk;

	/* For l2p pinning */
	struct ftl_l2p_pin_ctx		l2p_pin_ctx;

	/* Logical to physical mapping for this IO, number of entries equals to
	 * number of transfer blocks */
	ftl_addr			*map;

	struct spdk_bdev_io_wait_entry	bdev_io_wait;
};

/* */
struct ftl_rq_entry {
	/* Data payload of single entry (block) */
	void *io_payload;

	void *io_md;

	/*
	 * Physical address of block described by ftl_rq_entry.
	 * Valid after write command is completed (due to potential append reordering)
	 */
	ftl_addr addr;

	ftl_addr new_addr;

	/* Logical block address */
	uint64_t lba;

	/* Sequence id of original chunk where this user data was written to */
	uint64_t seq_id;

	/* Index of this entry within FTL request */
	const uint64_t index;

	struct {
		void *priv;
	} owner;

	/* If request issued in iterative way, it contains IO information */
	struct {
		struct ftl_band *band;
	} io;

	/* For l2p pinning */
	struct ftl_l2p_pin_ctx l2p_pin_ctx;

	struct {
		uint64_t offset_blocks;
		uint64_t num_blocks;
		uint64_t lba;
		struct spdk_bdev_io_wait_entry wait_entry;
	} bdev_io;
};

/*
 * Descriptor used for internal requests (compaction and reloc). May be split into multiple
 * IO requests (as valid blocks that need to be relocated may not be contiguous) - utilizing
 * the ftl_rq_entry array
 */
struct ftl_rq {
	struct spdk_ftl_dev *dev;

	/* Request queue entry */
	TAILQ_ENTRY(ftl_rq) qentry;

	/* Number of block within the request */
	uint64_t num_blocks;

	/* Extended metadata for IO. Its size is io_md_size * num_blocks */
	void *io_md;

	/* Size of extended metadata size for one entry */
	uint64_t io_md_size;

	/* Payload for IO */
	void *io_payload;

	/* Compaction rewrite valid data to which new chunk */
	struct ftl_nv_cache_chunk *rewriten_chunk;

	uint32_t tag;

	/* Request result status */
	bool success;

	/* Fields for owner of this request */
	struct {
		/* End request callback */
		void (*cb)(struct ftl_rq *rq);

		/* IO error request callback */
		void (*error)(struct ftl_rq *rq, struct ftl_band *band,
			      uint64_t idx, uint64_t count);

		/* Owner context */
		void *priv;

		/* This is compaction IO */
		bool compaction;
	} owner;

	/* Iterator fields for processing state of the request */
	struct {
		uint32_t idx;

		uint32_t count;

		/* Queue depth on this request */
		uint32_t qd;

		uint32_t remaining;
		int status;
	} iter;

	/* Private fields for issuing IO */
	struct {
		/* Request physical address, on IO completion set for append device */
		ftl_addr addr;

		/* Band to which IO is issued */
		struct ftl_band *band;

		struct spdk_bdev_io_wait_entry bdev_io_wait;
	} io;

	/* For writing P2L metadata */
	struct ftl_md_io_entry_ctx md_persist_entry_ctx;

	struct ftl_rq_entry entries[];
};

/* Used for reading/writing P2L map during runtime and recovery */
struct ftl_basic_rq {
	struct spdk_ftl_dev *dev;

	/* Request queue entry */
	TAILQ_ENTRY(ftl_basic_rq) qentry;

	/* Number of block within the request */
	uint64_t num_blocks;

	/* Payload for IO */
	void *io_payload;

	/* Request result status */
	bool success;

	/* Fields for owner of this request */
	struct {
		/* End request callback */
		void (*cb)(struct ftl_basic_rq *brq);

		/* Owner context */
		void *priv;
	} owner;

	/* Private fields for issuing IO */
	struct {
		/* Request physical address, on IO completion set for append device */
		ftl_addr addr;

		/* Band to which IO is issued */
		struct ftl_band *band;

		/* Chunk to which IO is issued */
		struct ftl_nv_cache_chunk *chunk;

		struct spdk_bdev_io_wait_entry bdev_io_wait;
	} io;
};

static inline bool
ftl_rq_entry_loop_assert(struct ftl_rq *rq, struct ftl_rq_entry *entry, uint32_t count)
{
	assert(entry >= rq->entries);
	assert(((uintptr_t)entry - (uintptr_t)rq->entries) % sizeof(*entry) == 0);
	assert(count <= rq->num_blocks);

	return true;
}

#define FTL_RQ_ENTRY_LOOP_FROM(rq, from, entry, count) \
	for ((entry) = (from); \
		(entry) < (&(rq)->entries[count]) && ftl_rq_entry_loop_assert(rq, entry, count); (entry)++)

#define FTL_RQ_ENTRY_LOOP(rq, entry, count) \
	FTL_RQ_ENTRY_LOOP_FROM(rq, (rq)->entries, entry, count)

void ftl_io_fail(struct ftl_io *io, int status);
void ftl_io_clear(struct ftl_io *io);
void ftl_io_inc_req(struct ftl_io *io);
void ftl_io_dec_req(struct ftl_io *io);
struct iovec *ftl_io_iovec(struct ftl_io *io);
uint64_t ftl_io_current_lba(const struct ftl_io *io);
uint64_t ftl_io_get_lba(const struct ftl_io *io, size_t offset);
void ftl_io_advance(struct ftl_io *io, size_t num_blocks);
size_t ftl_iovec_num_blocks(struct iovec *iov, size_t iov_cnt);
void *ftl_io_iovec_addr(struct ftl_io *io);
size_t ftl_io_iovec_len_left(struct ftl_io *io);
int ftl_io_init(struct spdk_io_channel *ioch, struct ftl_io *io, uint64_t lba,
		size_t num_blocks, struct iovec *iov, size_t iov_cnt, spdk_ftl_fn cb_fn,
		void *cb_arg, int type);
void ftl_io_complete(struct ftl_io *io);
void ftl_rq_del(struct ftl_rq *rq);
struct ftl_rq *ftl_rq_new(struct spdk_ftl_dev *dev, uint32_t io_md_size);
void ftl_rq_unpin(struct ftl_rq *rq);

static inline void
ftl_basic_rq_init(struct spdk_ftl_dev *dev, struct ftl_basic_rq *brq,
		  void *io_payload, uint64_t num_blocks)
{
	brq->dev = dev;
	brq->io_payload = io_payload;
	brq->num_blocks = num_blocks;
	brq->success = false;
}

static inline void
ftl_basic_rq_set_owner(struct ftl_basic_rq *brq, void (*cb)(struct ftl_basic_rq *brq), void *priv)
{
	brq->owner.cb = cb;
	brq->owner.priv = priv;
}

static inline struct ftl_rq *
ftl_rq_from_entry(struct ftl_rq_entry *entry)
{
	uint64_t idx = entry->index;
	struct ftl_rq *rq = SPDK_CONTAINEROF(entry, struct ftl_rq, entries[idx]);
	return rq;
}


static inline bool
ftl_io_done(const struct ftl_io *io)
{
	return io->req_cnt == 0 && io->pos == io->num_blocks;
}

#endif /* FTL_IO_H */
