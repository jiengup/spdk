#include "ftl_nvc_dev.h"
#include "ftl_nvc_op.h"

struct ftl_nv_cache_device_type nvc_bdev_vss = {
	.name = "single group",
	.features = {
	},
	.ops = {
		.is_bdev_compatible = is_bdev_compatible,
		.is_chunk_active = is_chunk_active,
		.get_user_io_tag = single_chunk_get_user_io_tag,
		.get_group_tag_for_compaction = get_single_group_tag_for_compaction,
		.md_layout_ops = {
			.region_create = md_region_create,
			.region_open = md_region_open,
		},
	}
};
FTL_NV_CACHE_DEVICE_TYPE_REGISTER(nvc_bdev_vss)
