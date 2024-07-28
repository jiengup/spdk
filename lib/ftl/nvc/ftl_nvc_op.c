#include "ftl_nvc_dev.h"
#include "ftl_core.h"
#include "ftl_layout.h"
#include "utils/ftl_layout_tracker_bdev.h"
#include "ftl_nvc_op.h"
#include <stdlib.h>
#include <time.h>

bool
is_bdev_compatible(struct spdk_ftl_dev *dev, struct spdk_bdev *bdev)
{
	if (!spdk_bdev_is_md_separate(bdev)) {
		/* It doesn't support separate metadata buffer IO */
		return false;
	}

	if (spdk_bdev_get_md_size(bdev) != sizeof(union ftl_md_vss)) {
		/* Bdev's metadata is invalid size */
		return false;
	}

	if (spdk_bdev_get_dif_type(bdev) != SPDK_DIF_DISABLE) {
		/* Unsupported DIF type used by bdev */
		return false;
	}

	if (ftl_md_xfer_blocks(dev) * spdk_bdev_get_md_size(bdev) > FTL_ZERO_BUFFER_SIZE) {
		FTL_ERRLOG(dev, "Zero buffer too small for bdev %s metadata transfer\n",
			   spdk_bdev_get_name(bdev));
		return false;
	}

	return true;
}

bool
is_chunk_active(struct spdk_ftl_dev *dev, uint64_t chunk_offset)
{
	const struct ftl_layout_tracker_bdev_region_props *reg_free = ftl_layout_tracker_bdev_insert_region(
				dev->nvc_layout_tracker, FTL_LAYOUT_REGION_TYPE_INVALID, 0, chunk_offset,
				dev->layout.nvc.chunk_data_blocks);

	assert(!reg_free || reg_free->type == FTL_LAYOUT_REGION_TYPE_FREE);
	return reg_free != NULL;
}

static void
md_region_setup(struct spdk_ftl_dev *dev, enum ftl_layout_region_type reg_type,
		struct ftl_layout_region *region)
{
	assert(region);
	region->type = reg_type;
	region->mirror_type = FTL_LAYOUT_REGION_TYPE_INVALID;
	region->name = ftl_md_region_name(reg_type);

	region->bdev_desc = dev->nv_cache.bdev_desc;
	region->ioch = dev->nv_cache.cache_ioch;
	region->vss_blksz = dev->nv_cache.md_size;
}

int
md_region_create(struct spdk_ftl_dev *dev, enum ftl_layout_region_type reg_type,
		 uint32_t reg_version, size_t reg_blks)
{
	const struct ftl_layout_tracker_bdev_region_props *reg_props;

	assert(reg_type < FTL_LAYOUT_REGION_TYPE_MAX);
	reg_blks = ftl_md_region_align_blocks(dev, reg_blks);

	reg_props = ftl_layout_tracker_bdev_add_region(dev->nvc_layout_tracker, reg_type, reg_version,
			reg_blks, 0);
	if (!reg_props) {
		return -1;
	}
	assert(reg_props->type == reg_type);
	assert(reg_props->ver == reg_version);
	assert(reg_props->blk_sz == reg_blks);
	assert(reg_props->blk_offs + reg_blks <= dev->layout.nvc.total_blocks);
	return 0;
}

int
md_region_open(struct spdk_ftl_dev *dev, enum ftl_layout_region_type reg_type, uint32_t reg_version,
	       size_t entry_size, size_t entry_count, struct ftl_layout_region *region)
{
	const struct ftl_layout_tracker_bdev_region_props *reg_search_ctx = NULL;
	uint64_t reg_blks = ftl_md_region_blocks(dev, entry_size * entry_count);

	assert(reg_type < FTL_LAYOUT_REGION_TYPE_MAX);

	while (true) {
		ftl_layout_tracker_bdev_find_next_region(dev->nvc_layout_tracker, reg_type, &reg_search_ctx);
		if (!reg_search_ctx || reg_search_ctx->ver == reg_version) {
			break;
		}
	}

	if (!reg_search_ctx || reg_search_ctx->blk_sz < reg_blks) {
		/* Region not found or insufficient space */
		return -1;
	}

	if (!region) {
		return 0;
	}

	md_region_setup(dev, reg_type, region);

	region->entry_size = entry_size / FTL_BLOCK_SIZE;
	region->num_entries = entry_count;

	region->current.version = reg_version;
	region->current.offset = reg_search_ctx->blk_offs;
	region->current.blocks = reg_search_ctx->blk_sz;

	return 0;
}

uint32_t
single_chunk_get_user_io_tag(struct spdk_ftl_dev *dev, uint64_t lba)
{
	assert(dev->nv_cache.traffic_group_num == 1);
    return 0;
}

uint32_t
random_chunk_get_user_io_tag(struct spdk_ftl_dev *dev, uint64_t lba)
{
    srand(time(NULL));
    uint32_t randn = rand();
    return randn % dev->nv_cache.traffic_group_num;
}

uint32_t
get_single_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
	assert(nv_cache->traffic_group_num == 1);
    return 0;
}

uint32_t
get_random_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq)
{
    srand(time(NULL));
    uint32_t randn = rand();
    return randn % nv_cache->traffic_group_num;
}