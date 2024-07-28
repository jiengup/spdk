/*   SPDX-License-Identifier: BSD-3-Clause
 *   Copyright 2023 Solidigm All Rights Reserved
 */

#include "spdk/stdinc.h"
#include "spdk/queue.h"
#include "spdk/log.h"
#include "spdk/string.h"

#include "ftl_nvc_dev.h"
#include "ftl_core.h"
#include "utils/ftl_defs.h"

static TAILQ_HEAD(, ftl_nv_cache_device_type) g_devs = TAILQ_HEAD_INITIALIZER(g_devs);
static pthread_mutex_t g_devs_mutex = PTHREAD_MUTEX_INITIALIZER;

static const struct ftl_nv_cache_device_type *
ftl_nv_cache_device_type_get_type(const char *name)
{
	struct ftl_nv_cache_device_type *entry;

	TAILQ_FOREACH(entry, &g_devs, internal.entry) {
		if (0 == strcmp(entry->name, name)) {
			return entry;
		}
	}

	return NULL;
}

static bool
config_select_type(const struct spdk_ftl_dev *dev, const struct ftl_nv_cache_device_type *type)
{
	return !strcmp(dev->conf.algo, type->name);
}

static bool
ftl_nv_cache_device_valid(const struct ftl_nv_cache_device_type *type)
{
	if (!type) {
		return false;
	}
	if (!type->ops.is_bdev_compatible) {
		return false;
	}
	if (!type->ops.is_chunk_active) {
		return false;
	}
	if (!type->ops.md_layout_ops.region_create) {
		return false;
	}
	if (!type->ops.md_layout_ops.region_open) {
		return false;
	}
	if (!type->name || strlen(type->name) <= 0) {
		return false;
	}
	if (!type->ops.get_user_io_tag) {
		return false;
	}
	if (!type->ops.get_group_tag_for_compaction) {
		return false;
	}
	return true;
	// return type && type->name && strlen(type->name) > 0;
}

void
ftl_nv_cache_device_register(struct ftl_nv_cache_device_type *type)
{
	if (!ftl_nv_cache_device_valid(type)) {
		SPDK_ERRLOG("NV cache device descriptor is invalid\n");
		ftl_abort();
	}

	pthread_mutex_lock(&g_devs_mutex);
	if (!ftl_nv_cache_device_type_get_type(type->name)) {
		TAILQ_INSERT_TAIL(&g_devs, type, internal.entry);
		SPDK_NOTICELOG("Registered NV cache device, name: %s\n", type->name);
	} else {
		SPDK_ERRLOG("Cannot register NV cache device, already exists, name: %s\n", type->name);
		ftl_abort();
	}

	pthread_mutex_unlock(&g_devs_mutex);
}

const struct ftl_nv_cache_device_type *
ftl_nv_cache_device_get_type_by_bdev(struct spdk_ftl_dev *dev, struct spdk_bdev *bdev)
{
	struct ftl_nv_cache_device_type *entry;
	const struct ftl_nv_cache_device_type *type = NULL;

	pthread_mutex_lock(&g_devs_mutex);
	uint32_t cnt = 0;
	TAILQ_FOREACH(entry, &g_devs, internal.entry) {
		if (entry->ops.is_bdev_compatible) {
			SPDK_NOTICELOG("dev algo: %s, entry name: %s\n", dev->conf.algo, entry->name);
			if (entry->ops.is_bdev_compatible(dev, bdev) && config_select_type(dev, entry)) {
				type = entry;
				cnt ++;
			}
		}
	}
	if (cnt > 1) {
		SPDK_ERRLOG("Found multiple nv cache device type registed!\n");
		type = NULL;
	}
	pthread_mutex_unlock(&g_devs_mutex);

	return type;
}
