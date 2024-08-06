#include "ftl_nvc_dev.h"
#include "ftl_nvc_op.h"

struct ftl_nv_cache_device_type nvc_bdev_greedy_vss = {
	.name = "single_group_greedy",
	.features = {
	},
	.ops = {
		.is_bdev_compatible = is_bdev_compatible,
		.is_chunk_active = is_chunk_active,
		.algo_info_init = algo_info_pass_init,
		.algo_info_update = algo_info_pass_update,
		.get_user_io_tag = single_chunk_get_user_io_tag,
		.get_group_tag_for_compaction = get_single_group_tag_for_compaction,
		.choose_chunk_for_compaction = greedy_choose_chunk_for_compaction,
		.md_layout_ops = {
			.region_create = md_region_create,
			.region_open = md_region_open,
		},
	}
};
FTL_NV_CACHE_DEVICE_TYPE_REGISTER(nvc_bdev_greedy_vss)

struct ftl_nv_cache_device_type nvc_bdev_cb_vss = {
	.name = "single_group_cb",
	.features = {
	},
	.ops = {
		.is_bdev_compatible = is_bdev_compatible,
		.is_chunk_active = is_chunk_active,
		.algo_info_init = algo_info_pass_init,
		.algo_info_update = algo_info_pass_update,
		.get_user_io_tag = single_chunk_get_user_io_tag,
		.get_group_tag_for_compaction = get_single_group_tag_for_compaction,
		.choose_chunk_for_compaction = costbenefit_choose_chunk_for_compaction,
		.md_layout_ops = {
			.region_create = md_region_create,
			.region_open = md_region_open,
		},
	}
};
FTL_NV_CACHE_DEVICE_TYPE_REGISTER(nvc_bdev_cb_vss)

struct ftl_nv_cache_device_type nvc_bdev_sepgc_greedy_vss = {
	.name = "sepgc_greedy",
	.features = {
	},
	.ops = {
		.is_bdev_compatible = is_bdev_compatible,
		.is_chunk_active = is_chunk_active,
		.algo_info_init = algo_info_pass_init,
		.algo_info_update = algo_info_pass_update,
		.get_user_io_tag = single_chunk_get_user_io_tag,
		.get_group_tag_for_compaction = get_sepgc_group_tag_for_compaction,
		.choose_chunk_for_compaction = greedy_choose_chunk_for_compaction,
		.md_layout_ops = {
			.region_create = md_region_create,
			.region_open = md_region_open,
		},
	}
};
FTL_NV_CACHE_DEVICE_TYPE_REGISTER(nvc_bdev_sepgc_greedy_vss)

struct ftl_nv_cache_device_type nvc_bdev_sepgc_cb_vss = {
	.name = "sepgc_cb",
	.features = {
	},
	.ops = {
		.is_bdev_compatible = is_bdev_compatible,
		.is_chunk_active = is_chunk_active,
		.algo_info_init = algo_info_pass_init,
		.algo_info_update = algo_info_pass_update,
		.get_user_io_tag = single_chunk_get_user_io_tag,
		.get_group_tag_for_compaction = get_sepgc_group_tag_for_compaction,
		.choose_chunk_for_compaction = costbenefit_choose_chunk_for_compaction,
		.md_layout_ops = {
			.region_create = md_region_create,
			.region_open = md_region_open,
		},
	}
};
FTL_NV_CACHE_DEVICE_TYPE_REGISTER(nvc_bdev_sepgc_cb_vss)

struct ftl_nv_cache_device_type nvc_bdev_mida_greedy_vss = {
	.name = "mida_greedy",
	.features = {
	},
	.ops = {
		.is_bdev_compatible = is_bdev_compatible,
		.is_chunk_active = is_chunk_active,
		.algo_info_init = algo_info_pass_init,
		.algo_info_update = algo_info_pass_update,
		.get_user_io_tag = single_chunk_get_user_io_tag,
		.get_group_tag_for_compaction = get_mida_group_tag_for_compaction,
		.choose_chunk_for_compaction = greedy_choose_chunk_for_compaction,
		.md_layout_ops = {
			.region_create = md_region_create,
			.region_open = md_region_open,
		},
	}
};
FTL_NV_CACHE_DEVICE_TYPE_REGISTER(nvc_bdev_mida_greedy_vss)

struct ftl_nv_cache_device_type nvc_bdev_mida_cb_vss = {
	.name = "mida_cb",
	.features = {
	},
	.ops = {
		.is_bdev_compatible = is_bdev_compatible,
		.is_chunk_active = is_chunk_active,
		.algo_info_init = algo_info_pass_init,
		.algo_info_update = algo_info_pass_update,
		.get_user_io_tag = single_chunk_get_user_io_tag,
		.get_group_tag_for_compaction = get_mida_group_tag_for_compaction,
		.choose_chunk_for_compaction = costbenefit_choose_chunk_for_compaction,
		.md_layout_ops = {
			.region_create = md_region_create,
			.region_open = md_region_open,
		},
	}
};
FTL_NV_CACHE_DEVICE_TYPE_REGISTER(nvc_bdev_mida_cb_vss)

struct ftl_nv_cache_device_type nvc_bdev_sepbit_greedy_vss = {
	.name = "sepbit_greedy",
	.features = {
	},
	.ops = {
		.is_bdev_compatible = is_bdev_compatible,
		.is_chunk_active = is_chunk_active,
		.algo_info_init = algo_info_sepbit_init,
		.algo_info_update = algo_info_sepbit_update,
		.get_user_io_tag = sepbit_chunk_get_user_io_tag,
		.get_group_tag_for_compaction = get_sepbit_group_tag_for_compaction,
		.choose_chunk_for_compaction = greedy_choose_chunk_for_compaction,
		.md_layout_ops = {
			.region_create = md_region_create,
			.region_open = md_region_open,
		},
	}
};
FTL_NV_CACHE_DEVICE_TYPE_REGISTER(nvc_bdev_sepbit_greedy_vss)

struct ftl_nv_cache_device_type nvc_bdev_sepbit_cb_vss = {
	.name = "sepbit_cb",
	.features = {
	},
	.ops = {
		.is_bdev_compatible = is_bdev_compatible,
		.is_chunk_active = is_chunk_active,
		.algo_info_init = algo_info_sepbit_init,
		.algo_info_update = algo_info_sepbit_update,
		.get_user_io_tag = sepbit_chunk_get_user_io_tag,
		.get_group_tag_for_compaction = get_sepbit_group_tag_for_compaction,
		.choose_chunk_for_compaction = costbenefit_choose_chunk_for_compaction,
		.md_layout_ops = {
			.region_create = md_region_create,
			.region_open = md_region_open,
		},
	}
};
FTL_NV_CACHE_DEVICE_TYPE_REGISTER(nvc_bdev_sepbit_cb_vss)
