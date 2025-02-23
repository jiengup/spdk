#include "ftl_nv_cache.h"
#include "ftl_nvc_dev.h"
#include "ftl_core.h"
#include "ftl_layout.h"
#include "utils/ftl_layout_tracker_bdev.h"
#include "ftl_nvc_op.h"
#include <stdlib.h>
#include <time.h>

// ----------------------algo init------------------------------------
void
algo_info_pass_init(struct ftl_nv_cache *nv_cache)
{
	return;
}

void
algo_info_sepbit_init(struct ftl_nv_cache *nv_cache)
{
	nv_cache->sepbit_info.threshold = UINT64_MAX;
	nv_cache->sepbit_info.threshold_total = 0;
	nv_cache->sepbit_info.count = 0;
}

// -------------------algo info update--------------------------------
void
algo_info_pass_update(struct ftl_nv_cache *nv_cache, struct ftl_nv_cache_chunk *chunk)
{
	return;
}

void
algo_info_sepbit_update(struct ftl_nv_cache *nv_cache, struct ftl_nv_cache_chunk *chunk)
{
	struct spdk_ftl_dev *dev = SPDK_CONTAINEROF(nv_cache, struct spdk_ftl_dev, nv_cache);

	assert(chunk->md->tag != FTL_TAG_INVALID);
	assert(chunk->md->timestamp != UINT64_MAX);

	if (chunk->md->tag == 0) {
		nv_cache->sepbit_info.count ++;
		nv_cache->sepbit_info.threshold_total += (ftl_get_timestamp(dev) - chunk->md->timestamp);
	}
	if (nv_cache->sepbit_info.count == FTL_NV_CACHE_SEPBIT_PERIOD) {
		nv_cache->sepbit_info.threshold = nv_cache->sepbit_info.threshold_total / nv_cache->sepbit_info.count;
		nv_cache->sepbit_info.count = 0;
		nv_cache->sepbit_info.threshold_total = 0;
	}
}

// -------------------user io grouping--------------------------------
uint8_t
single_chunk_get_user_io_tag(struct spdk_ftl_dev *dev, struct ftl_io *io)
{
    return 0;
}

uint8_t
random_chunk_get_user_io_tag(struct spdk_ftl_dev *dev, struct ftl_io *io)
{
    srand(time(NULL));
    uint32_t randn = rand();
    return randn % dev->nv_cache.traffic_group_num;
}

uint8_t
get_user_io_tag_th2(struct spdk_ftl_dev *dev, struct ftl_io *io) 
{
	struct ftl_nv_cache *nv_cache = &dev->nv_cache;
	uint64_t *lba_timestamp_table = nv_cache->lba_timestamp_table;

	uint64_t lba = ftl_io_get_lba(io, 0);

	uint64_t i;

	uint64_t unseen_lbas = 0;
	uint64_t livespan = 0;

	for (i = 0; i < io->num_blocks; ++i, lba++) {
		assert(lba < dev->num_lbas);
		if (lba_timestamp_table[lba] == UINT64_MAX) {
			unseen_lbas ++;
		} else {
			assert(io->timestamp > lba_timestamp_table[lba]);
			livespan += io->timestamp - lba_timestamp_table[lba];
		}
	}

	if (unseen_lbas > io->num_blocks / 2) {
		return 1;
	}

	assert(io->num_blocks != unseen_lbas);
	livespan /= (io->num_blocks - unseen_lbas);

	if (livespan < nv_cache->sepbit_info.threshold) {
		return 0;
	} else {
		return 1;
	}
}

uint8_t
get_user_io_tag_th4(struct spdk_ftl_dev *dev, struct ftl_io *io) 
{
	struct ftl_nv_cache *nv_cache = &dev->nv_cache;
	uint64_t *lba_timestamp_table = nv_cache->lba_timestamp_table;

	uint64_t lba = ftl_io_get_lba(io, 0);

	uint64_t i;

	uint64_t unseen_lbas = 0;
	uint64_t livespan = 0;

	for (i = 0; i < io->num_blocks; ++i, lba++) {
		assert(lba < dev->num_lbas);
		if (lba_timestamp_table[lba] == UINT64_MAX) {
			unseen_lbas ++;
		} else {
			assert(io->timestamp > lba_timestamp_table[lba]);
			livespan += io->timestamp - lba_timestamp_table[lba];
		}
	}

	if (unseen_lbas > io->num_blocks / 2) {
		return 0;
	}

	assert(io->num_blocks != unseen_lbas);
	livespan /= (io->num_blocks - unseen_lbas);

	if (livespan < nv_cache->sepbit_info.threshold / 8) {
		return 0;
	} else if (livespan < nv_cache->sepbit_info.threshold / 4) {
		return 1;
	} else if (livespan < nv_cache->sepbit_info.threshold / 2) {
		return 2;
	} else {
		return 3;
	}
}

// --------------------GC IO grouping--------------------------------
uint8_t
get_single_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
    return 0;
}

uint8_t
get_random_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
    srand(time(NULL));
    uint32_t randn = rand();
    return randn % nv_cache->traffic_group_num;
}

uint8_t
th2_get_sepbit2_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
	struct ftl_rq_entry *entry;
	struct ftl_nv_cache_chunk *chunk;
	struct spdk_ftl_dev *dev = SPDK_CONTAINEROF(nv_cache, struct spdk_ftl_dev, nv_cache);
	uint64_t lba;

	// assert(!strcmp(dev->conf.algo, "sepbit"));
	assert(nv_cache->traffic_group_num == 4);
	assert(rq->iter.count);

	uint32_t cnt[4];
	for (int i = 2; i < 4; i++) {
		cnt[i] = 0;
	}
	uint32_t validcnt = 0;
	
	FTL_RQ_ENTRY_LOOP(rq, entry, rq->iter.count) {
		lba = entry->lba;
		assert(lba < dev->num_lbas);
		if (lba == FTL_LBA_INVALID) {
			continue;
		}
		validcnt ++;
		chunk = (struct ftl_nv_cache_chunk*)(entry->owner.priv);
		assert(chunk != NULL);
		assert(chunk->md->tag != FTL_TAG_INVALID);
		if (chunk->md->tag == 0) {
			cnt[2] ++;
		} else {
			cnt[3] ++;
		}
	}
	
	uint32_t max_cnt = 0;
	uint8_t tag = 0;
	for (int i = 2; i < 4; i++) {
		if (max_cnt < cnt[i]) {
			max_cnt = cnt[i];
			tag = i;
		}
	}
	return tag;
}

uint8_t
th2_get_sepbit4_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
	struct ftl_rq_entry *entry;
	struct ftl_nv_cache_chunk *chunk;
	struct spdk_ftl_dev *dev = SPDK_CONTAINEROF(nv_cache, struct spdk_ftl_dev, nv_cache);
	uint64_t *lba_timestamp_table = nv_cache->lba_timestamp_table;
	uint64_t lba;

	// assert(!strcmp(dev->conf.algo, "sepbit"));
	assert(nv_cache->traffic_group_num == 6);
	assert(rq->iter.count);

	uint32_t cnt[6];
	for (int i = 2; i < 6; i++) {
		cnt[i] = 0;
	}
	uint32_t validcnt = 0;
	
	FTL_RQ_ENTRY_LOOP(rq, entry, rq->iter.count) {
		lba = entry->lba;
		assert(lba < dev->num_lbas);
		if (lba == FTL_LBA_INVALID) {
			continue;
		}
		validcnt ++;
		chunk = (struct ftl_nv_cache_chunk*)(entry->owner.priv);
		assert(chunk != NULL);
		assert(chunk->md->tag != FTL_TAG_INVALID);
		if (chunk->md->tag == 0) {
			cnt[2] ++;
		} else {
			assert(lba_timestamp_table[lba] != UINT64_MAX);
			uint64_t livespan = ftl_get_timestamp(dev);
			assert(lba_timestamp_table[lba] < livespan);
			livespan -= lba_timestamp_table[lba];
			
			if (livespan < 4 * nv_cache->sepbit_info.threshold) {
				cnt[3] ++;
			} else if (livespan < 16 * nv_cache->sepbit_info.threshold) {
				cnt[4] ++;
			} else {
				cnt[5] ++;
			}
		}
	}
	
	if (cnt[2] == validcnt) {
		return 2;
	}
	
	uint32_t max_cnt = 0;
	uint8_t tag = 0;
	for (int i = 3; i < 6; i++) {
		if (max_cnt < cnt[i]) {
			max_cnt = cnt[i];
			tag = i;
		}
	}
	return tag;
}

uint8_t
th4_get_sepbit4_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
	struct ftl_rq_entry *entry;
	struct ftl_nv_cache_chunk *chunk;
	struct spdk_ftl_dev *dev = SPDK_CONTAINEROF(nv_cache, struct spdk_ftl_dev, nv_cache);
	uint64_t *lba_timestamp_table = nv_cache->lba_timestamp_table;
	uint64_t lba;

	// assert(!strcmp(dev->conf.algo, "sepbit"));
	assert(nv_cache->traffic_group_num == 8);
	assert(rq->iter.count);

	uint32_t cnt[8];
	for (int i = 4; i < 8; i++) {
		cnt[i] = 0;
	}
	uint32_t validcnt = 0;
	
	FTL_RQ_ENTRY_LOOP(rq, entry, rq->iter.count) {
		lba = entry->lba;
		assert(lba < dev->num_lbas);
		if (lba == FTL_LBA_INVALID) {
			continue;
		}
		validcnt ++;
		chunk = (struct ftl_nv_cache_chunk*)(entry->owner.priv);
		assert(chunk != NULL);
		assert(chunk->md->tag != FTL_TAG_INVALID);
		if (chunk->md->tag < 2) {
			cnt[4] ++;
		} else {
			assert(lba_timestamp_table[lba] != UINT64_MAX);
			uint64_t livespan = ftl_get_timestamp(dev);
			assert(lba_timestamp_table[lba] < livespan);
			livespan -= lba_timestamp_table[lba];
			
			if (livespan < 4 * nv_cache->sepbit_info.threshold) {
				cnt[5] ++;
			} else if (livespan < 16 * nv_cache->sepbit_info.threshold) {
				cnt[6] ++;
			} else {
				cnt[7] ++;
			}
		}
	}
	
	uint32_t max_cnt = 0;
	uint8_t tag = 0;
	for (int i = 4; i < 8; i++) {
		if (max_cnt < cnt[i]) {
			max_cnt = cnt[i];
			tag = i;
		}
	}
	return tag;
}

uint8_t
th4_get_sepbit6_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
	struct ftl_rq_entry *entry;
	struct ftl_nv_cache_chunk *chunk;
	struct spdk_ftl_dev *dev = SPDK_CONTAINEROF(nv_cache, struct spdk_ftl_dev, nv_cache);
	uint64_t *lba_timestamp_table = nv_cache->lba_timestamp_table;
	uint64_t lba;

	// assert(!strcmp(dev->conf.algo, "sepbit"));
	assert(nv_cache->traffic_group_num == 10);
	assert(rq->iter.count);

	uint32_t cnt[10];
	for (int i = 4; i < 10; i++) {
		cnt[i] = 0;
	}
	uint32_t validcnt = 0;
	
	FTL_RQ_ENTRY_LOOP(rq, entry, rq->iter.count) {
		lba = entry->lba;
		assert(lba < dev->num_lbas);
		if (lba == FTL_LBA_INVALID) {
			continue;
		}
		validcnt ++;
		chunk = (struct ftl_nv_cache_chunk*)(entry->owner.priv);
		assert(chunk != NULL);
		assert(chunk->md->tag != FTL_TAG_INVALID);
		if (chunk->md->tag < 2) {
			cnt[4] ++;
		} else {
			assert(lba_timestamp_table[lba] != UINT64_MAX);
			uint64_t livespan = ftl_get_timestamp(dev);
			assert(lba_timestamp_table[lba] < livespan);
			livespan -= lba_timestamp_table[lba];
			
			if (livespan < 2 * nv_cache->sepbit_info.threshold) {
				cnt[5] ++;
			} else if (livespan < 4 * nv_cache->sepbit_info.threshold) {
				cnt[6] ++;
			} else if (livespan < 8 * nv_cache->sepbit_info.threshold) {
				cnt[7] ++;
			} else if(livespan < 16 * nv_cache->sepbit_info.threshold) {
				cnt[8] ++;
			} else {
				cnt[9] ++;
			}
		}
	}
	
	uint32_t max_cnt = 0;
	uint8_t tag = 0;
	for (int i = 4; i < 10; i++) {
		if (max_cnt < cnt[i]) {
			max_cnt = cnt[i];
			tag = i;
		}
	}
	return tag;
}

uint8_t
th2_get_mida2_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
	struct ftl_rq_entry *entry;
	struct ftl_nv_cache_chunk *chunk;
	uint64_t lba;

	assert(rq->iter.count);
	assert(nv_cache->traffic_group_num == 4);

	uint32_t cnt[4];
	for (int i = 2; i < 4; i++) {
		cnt[i] = 0;
	}
	
	FTL_RQ_ENTRY_LOOP(rq, entry, rq->iter.count) {
		lba = entry->lba;
		if (lba == FTL_LBA_INVALID) {
			continue;
		}
		chunk = (struct ftl_nv_cache_chunk*)(entry->owner.priv);
		assert(chunk != NULL);
		assert(chunk->md->tag != FTL_TAG_INVALID);
		if (chunk->md->tag == 0) {
			cnt[2] ++;
		} else if(chunk->md->tag == 3) {
			cnt[3] ++;
		} else {
			cnt[chunk->md->tag+1] ++;
		}
	}

	uint32_t max_cnt = 0;
	uint8_t tag = 0;
	for (int i = 2; i < 4; i++) {
		if (max_cnt < cnt[i]) {
			max_cnt = cnt[i];
			tag = i;
		}
	}
	return tag;
}

uint8_t
th2_get_mida4_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
	struct ftl_rq_entry *entry;
	struct ftl_nv_cache_chunk *chunk;
	uint64_t lba;

	assert(rq->iter.count);
	assert(nv_cache->traffic_group_num == 6);

	uint32_t cnt[6];
	for (int i = 2; i < 6; i++) {
		cnt[i] = 0;
	}
	
	FTL_RQ_ENTRY_LOOP(rq, entry, rq->iter.count) {
		lba = entry->lba;
		if (lba == FTL_LBA_INVALID) {
			continue;
		}
		chunk = (struct ftl_nv_cache_chunk*)(entry->owner.priv);
		assert(chunk != NULL);
		assert(chunk->md->tag != FTL_TAG_INVALID);
		if (chunk->md->tag == 0) {
			cnt[2] ++;
		} else if(chunk->md->tag == 5) {
			cnt[5] ++;
		} else {
			cnt[chunk->md->tag+1] ++;
		}
	}

	uint32_t max_cnt = 0;
	uint8_t tag = 0;
	for (int i = 2; i < 6; i++) {
		if (max_cnt < cnt[i]) {
			max_cnt = cnt[i];
			tag = i;
		}
	}
	return tag;
}

uint8_t
th4_get_mida4_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
	struct ftl_rq_entry *entry;
	struct ftl_nv_cache_chunk *chunk;
	uint64_t lba;

	assert(rq->iter.count);
	assert(nv_cache->traffic_group_num == 8);

	uint32_t cnt[8];
	for (int i = 4; i < 8; i++) {
		cnt[i] = 0;
	}
	
	FTL_RQ_ENTRY_LOOP(rq, entry, rq->iter.count) {
		lba = entry->lba;
		if (lba == FTL_LBA_INVALID) {
			continue;
		}
		chunk = (struct ftl_nv_cache_chunk*)(entry->owner.priv);
		assert(chunk != NULL);
		assert(chunk->md->tag != FTL_TAG_INVALID);
		if (chunk->md->tag < 3) {
			cnt[4] ++;
		} else if(chunk->md->tag == 7) {
			cnt[7] ++;
		} else {
			cnt[chunk->md->tag+1] ++;
		}
	}

	uint32_t max_cnt = 0;
	uint8_t tag = 0;
	for (int i = 4; i < 8; i++) {
		if (max_cnt < cnt[i]) {
			max_cnt = cnt[i];
			tag = i;
		}
	}
	return tag;
}

uint8_t
th4_get_mida6_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
	struct ftl_rq_entry *entry;
	struct ftl_nv_cache_chunk *chunk;
	uint64_t lba;

	assert(rq->iter.count);
	assert(nv_cache->traffic_group_num == 10);

	uint32_t cnt[10];
	for (int i = 4; i < 10; i++) {
		cnt[i] = 0;
	}
	
	FTL_RQ_ENTRY_LOOP(rq, entry, rq->iter.count) {
		lba = entry->lba;
		if (lba == FTL_LBA_INVALID) {
			continue;
		}
		chunk = (struct ftl_nv_cache_chunk*)(entry->owner.priv);
		assert(chunk != NULL);
		assert(chunk->md->tag != FTL_TAG_INVALID);
		if (chunk->md->tag < 3) {
			cnt[4] ++;
		} else if(chunk->md->tag == 9) {
			cnt[9] ++;
		} else {
			cnt[chunk->md->tag+1] ++;
		}
	}

	uint32_t max_cnt = 0;
	uint8_t tag = 0;
	for (int i = 4; i < 10; i++) {
		if (max_cnt < cnt[i]) {
			max_cnt = cnt[i];
			tag = i;
		}
	}
	return tag;
}

uint8_t
get_sepgc_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
	struct spdk_ftl_dev *dev = SPDK_CONTAINEROF(nv_cache, struct spdk_ftl_dev, nv_cache);
	// assert(!strcmp(dev->conf.algo, "sepgc"));
	assert(rq->iter.count);

	return 1;
}

// -------------------GC choose segment------------------------------
struct ftl_nv_cache_chunk*
fifo_choose_chunk_for_compaction(struct ftl_nv_cache *nv_cache)
{
	struct ftl_nv_cache_chunk *chunk;
	assert(!TAILQ_EMPTY(&nv_cache->chunk_full_list));
	chunk = TAILQ_FIRST(&nv_cache->chunk_full_list);
	assert(chunk != NULL);
	return chunk;
}

struct ftl_nv_cache_chunk*
greedy_choose_chunk_for_compaction(struct ftl_nv_cache *nv_cache)
{
	assert(!TAILQ_EMPTY(&nv_cache->chunk_full_list));
	struct spdk_ftl_dev *dev = SPDK_CONTAINEROF(nv_cache, struct spdk_ftl_dev, nv_cache);
	struct ftl_nv_cache_chunk *chunk;
	struct ftl_nv_cache_chunk *nchunk, *tchunk;
	uint64_t min_valid = UINT64_MAX;
	TAILQ_FOREACH_SAFE(nchunk, &nv_cache->chunk_full_list, entry, tchunk) {
		uint64_t start = ftl_addr_from_nvc_offset(dev, nchunk->offset);
		uint64_t end = start + (nv_cache->chunk_blocks - nv_cache->tail_md_chunk_blocks);
		assert(nchunk->md->valid_count == ftl_bitmap_count_set_range(dev->valid_map, start, end));
		uint64_t valid_count = nchunk->md->valid_count;
		if (valid_count < min_valid) {
			min_valid = valid_count;
			chunk = nchunk;
		}
	}
	assert(chunk != NULL);
	return chunk;
}

struct ftl_nv_cache_chunk* 
costbenefit_choose_chunk_for_compaction(struct ftl_nv_cache *nv_cache)
{
	assert(!TAILQ_EMPTY(&nv_cache->chunk_full_list));
	struct spdk_ftl_dev *dev = SPDK_CONTAINEROF(nv_cache, struct spdk_ftl_dev, nv_cache);
	struct ftl_nv_cache_chunk *chunk;
	struct ftl_nv_cache_chunk *nchunk, *tchunk, *best_chunk;
	double max_benefit = 0;
	TAILQ_FOREACH_SAFE(nchunk, &nv_cache->chunk_full_list, entry, tchunk) {
		// TODO(fix)
		uint64_t age = ftl_get_timestamp(dev) - nchunk->md->last_invalid_timestamp + 1;
		uint64_t garbage = nv_cache->chunk_blocks - nv_cache->tail_md_chunk_blocks - nchunk->md->valid_count;
		if (garbage == nv_cache->chunk_blocks - nv_cache->tail_md_chunk_blocks)
			return nchunk;
		double gp = (double)garbage / (double)(nv_cache->chunk_blocks - nv_cache->tail_md_chunk_blocks);
		double benefit = (gp * gp / (1-gp)) * (double)age;
		if (benefit > max_benefit) {
			max_benefit = benefit;
			best_chunk = nchunk;
		}
	}
	assert(best_chunk != NULL);
	return best_chunk;
}