#include "ftl_core.h"
#include "ftl_nv_cache.h"
#include "ftl_io.h"
#include "ftl_internal.h"

/* ----------------------------算法初始化----------------------------------------*/
void algo_info_pass_init(struct ftl_nv_cache *nv_cache);
void algo_info_sepbit_init(struct ftl_nv_cache *nv_cache);
/* ----------------------------维护算法统计----------------------------------------*/
void algo_info_pass_update(struct ftl_nv_cache *nv_cache, struct ftl_nv_cache_chunk *chunk);
void algo_info_sepbit_update(struct ftl_nv_cache *nv_cache, struct ftl_nv_cache_chunk *chunk);

/* ---------------------------分流算法-------------------------------------------*/
uint8_t single_chunk_get_user_io_tag(struct spdk_ftl_dev *dev, struct ftl_io *io);
uint8_t random_chunk_get_user_io_tag(struct spdk_ftl_dev *dev, struct ftl_io *io);
uint8_t sepbit_chunk_get_user_io_tag(struct spdk_ftl_dev *dev, struct ftl_io *io);

/* -----------------------compaction重写分流--------------------------------------*/
uint8_t get_single_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq);
uint8_t get_random_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq);
uint8_t get_sepbit_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq);
uint8_t get_mida_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq);
uint8_t get_sepgc_group_tag_for_compaction(struct ftl_nv_cache *nv_cache, struct ftl_rq *rq);

/* -----------------------compaction回收选段--------------------------------------*/
struct ftl_nv_cache_chunk* fifo_choose_chunk_for_compaction(struct ftl_nv_cache *nv_cache);
struct ftl_nv_cache_chunk* greedy_choose_chunk_for_compaction(struct ftl_nv_cache *nv_cache);
struct ftl_nv_cache_chunk* costbenefit_choose_chunk_for_compaction(struct ftl_nv_cache *nv_cache);