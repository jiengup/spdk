#include "ftl_core.h"
#include "ftl_nv_cache.h"
#include "ftl_io.h"
#include "ftl_internal.h"

bool is_bdev_compatible(struct spdk_ftl_dev *dev, struct spdk_bdev *bdev);
bool is_chunk_active(struct spdk_ftl_dev *dev, uint64_t chunk_offset);
int md_region_create(struct spdk_ftl_dev *dev, enum ftl_layout_region_type reg_type,
		    uint32_t reg_version, size_t reg_blks);
int md_region_open(struct spdk_ftl_dev *dev, enum ftl_layout_region_type reg_type, uint32_t reg_version,
	        size_t entry_size, size_t entry_count, struct ftl_layout_region *region);

/* ---------------------------分流算法-------------------------------------------*/
uint32_t single_chunk_get_user_io_tag(struct spdk_ftl_dev *dev, uint64_t lba);
uint32_t random_chunk_get_user_io_tag(struct spdk_ftl_dev *dev, uint64_t lba);

/* -----------------------compaction重写分流--------------------------------------*/
uint32_t get_single_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq);
uint32_t get_random_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq);

/* -----------------------compaction回收选段--------------------------------------*/