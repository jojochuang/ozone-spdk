/*
 * Copyright (c) 2024 Wei-Chiu Chuang
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bdev_ozone.h"
#include "ozone_bdev.h"

#include "spdk/log.h"
#include "spdk/string.h"
#include "spdk/rpc.h"
#include "spdk/util.h"
#include "spdk/thread.h"

#include <hdfs.h>

/* Global list of Ozone bdevs */
static TAILQ_HEAD(, ozone_bdev) g_ozone_bdev_head = TAILQ_HEAD_INITIALIZER(g_ozone_bdev_head);
static pthread_mutex_t g_ozone_bdev_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Forward declarations for bdev function table
 */
static int bdev_ozone_destruct(void *ctx);
static void bdev_ozone_submit_request(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io);
static bool bdev_ozone_io_type_supported(void *ctx, enum spdk_bdev_io_type io_type);
static struct spdk_io_channel *bdev_ozone_get_io_channel(void *ctx);
static int bdev_ozone_dump_info_json(void *ctx, struct spdk_json_write_ctx *w);
static void bdev_ozone_write_config_json(struct spdk_bdev *bdev, struct spdk_json_write_ctx *w);

/*
 * Bdev function table
 */
static const struct spdk_bdev_fn_table ozone_fn_table = {
	.destruct = bdev_ozone_destruct,
	.submit_request = bdev_ozone_submit_request,
	.io_type_supported = bdev_ozone_io_type_supported,
	.get_io_channel = bdev_ozone_get_io_channel,
	.dump_info_json = bdev_ozone_dump_info_json,
	.write_config_json = bdev_ozone_write_config_json,
};

/*
 * Module finish callback
 */
static void
bdev_ozone_finish_cb(void)
{
	/* Cleanup will be handled by bdev layer calling destruct */
}

/*
 * Bdev module definition
 */
static struct spdk_bdev_module ozone_if = {
	.name = "ozone",
	.module_init = bdev_ozone_initialize,
	.module_fini = bdev_ozone_finish,
	.async_fini = false,
};

SPDK_BDEV_MODULE_REGISTER(ozone, &ozone_if)

struct spdk_bdev_module *
bdev_ozone_get_module(void)
{
	return &ozone_if;
}

/*
 * I/O channel creation
 */
static int
bdev_ozone_create_cb(void *io_device, void *ctx_buf)
{
	struct ozone_io_channel *ch = ctx_buf;
	
	/* Initialize per-thread HDFS context */
	ch->hdfs_context = NULL;
	
	SPDK_NOTICELOG("Ozone I/O channel created\n");
	return 0;
}

/*
 * I/O channel destruction
 */
static void
bdev_ozone_destroy_cb(void *io_device, void *ctx_buf)
{
	struct ozone_io_channel *ch = ctx_buf;
	
	/* Cleanup HDFS context if needed */
	if (ch->hdfs_context != NULL) {
		/* TODO: Cleanup HDFS context */
	}
	
	SPDK_NOTICELOG("Ozone I/O channel destroyed\n");
}

/*
 * Get I/O channel
 */
static struct spdk_io_channel *
bdev_ozone_get_io_channel(void *ctx)
{
	struct ozone_bdev *ozone_bdev = ctx;
	
	return spdk_get_io_channel(ozone_bdev);
}

/*
 * Check if I/O type is supported
 */
static bool
bdev_ozone_io_type_supported(void *ctx, enum spdk_bdev_io_type io_type)
{
	switch (io_type) {
	case SPDK_BDEV_IO_TYPE_READ:
	case SPDK_BDEV_IO_TYPE_WRITE:
	case SPDK_BDEV_IO_TYPE_UNMAP:
	case SPDK_BDEV_IO_TYPE_FLUSH:
		return true;
	default:
		return false;
	}
}

/*
 * Submit I/O request
 */
static void
bdev_ozone_submit_request(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	struct ozone_io_channel *ozone_ch = spdk_io_channel_get_ctx(ch);
	int rc = 0;
	
	switch (bdev_io->type) {
	case SPDK_BDEV_IO_TYPE_READ:
		/* TODO: Implement read from Ozone via libhdfs */
		SPDK_NOTICELOG("READ: offset=%lu, length=%lu\n",
			       bdev_io->u.bdev.offset_blocks,
			       bdev_io->u.bdev.num_blocks);
		/* For now, just return zeros */
		rc = 0;
		break;
		
	case SPDK_BDEV_IO_TYPE_WRITE:
		/* TODO: Implement write to Ozone via libhdfs */
		SPDK_NOTICELOG("WRITE: offset=%lu, length=%lu\n",
			       bdev_io->u.bdev.offset_blocks,
			       bdev_io->u.bdev.num_blocks);
		rc = 0;
		break;
		
	case SPDK_BDEV_IO_TYPE_UNMAP:
		/* TODO: Implement unmap/trim */
		SPDK_NOTICELOG("UNMAP: offset=%lu, length=%lu\n",
			       bdev_io->u.bdev.offset_blocks,
			       bdev_io->u.bdev.num_blocks);
		rc = 0;
		break;
		
	case SPDK_BDEV_IO_TYPE_FLUSH:
		/* TODO: Implement flush */
		SPDK_NOTICELOG("FLUSH\n");
		rc = 0;
		break;
		
	default:
		rc = -ENOTSUP;
		break;
	}
	
	if (rc == 0) {
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_SUCCESS);
	} else {
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
	}
}

/*
 * Destruct bdev
 */
static int
bdev_ozone_destruct(void *ctx)
{
	struct ozone_bdev *ozone_bdev = ctx;
	
	pthread_mutex_lock(&g_ozone_bdev_mutex);
	TAILQ_REMOVE(&g_ozone_bdev_head, ozone_bdev, link);
	pthread_mutex_unlock(&g_ozone_bdev_mutex);
	
	/* Cleanup HDFS handle */
	if (ozone_bdev->hdfs_handle != NULL) {
		hdfsDisconnect((hdfsFS)ozone_bdev->hdfs_handle);
	}
	
	free(ozone_bdev->ozone_uri);
	spdk_io_device_unregister(ozone_bdev, NULL);
	free(ozone_bdev);
	
	SPDK_NOTICELOG("Ozone bdev destructed\n");
	return 0;
}

/*
 * Dump bdev info as JSON
 */
static int
bdev_ozone_dump_info_json(void *ctx, struct spdk_json_write_ctx *w)
{
	struct ozone_bdev *ozone_bdev = ctx;
	
	spdk_json_write_named_object_begin(w, "ozone");
	spdk_json_write_named_string(w, "ozone_uri", ozone_bdev->ozone_uri);
	spdk_json_write_named_uint32(w, "chunk_size", ozone_bdev->chunk_size);
	spdk_json_write_object_end(w);
	
	return 0;
}

/*
 * Write bdev config as JSON
 */
static void
bdev_ozone_write_config_json(struct spdk_bdev *bdev, struct spdk_json_write_ctx *w)
{
	struct ozone_bdev *ozone_bdev = SPDK_CONTAINEROF(bdev, struct ozone_bdev, bdev);
	
	spdk_json_write_object_begin(w);
	spdk_json_write_named_string(w, "method", "ozone_bdev_create");
	
	spdk_json_write_named_object_begin(w, "params");
	spdk_json_write_named_string(w, "name", bdev->name);
	spdk_json_write_named_string(w, "ozone_uri", ozone_bdev->ozone_uri);
	spdk_json_write_named_uint64(w, "size", ozone_bdev->size_bytes);
	spdk_json_write_named_uint32(w, "block_size", ozone_bdev->block_size);
	spdk_json_write_named_uint32(w, "chunk_size", ozone_bdev->chunk_size);
	spdk_json_write_object_end(w);
	
	spdk_json_write_object_end(w);
}

/*
 * Module initialization
 */
int
bdev_ozone_initialize(void)
{
	SPDK_NOTICELOG("Ozone bdev module initialized\n");
	return 0;
}

/*
 * Module finish
 */
void
bdev_ozone_finish(void)
{
	bdev_ozone_finish_cb();
	SPDK_NOTICELOG("Ozone bdev module finished\n");
}

/*
 * Create an Ozone bdev
 */
int
ozone_bdev_create(const struct ozone_bdev_config *config)
{
	struct ozone_bdev *ozone_bdev;
	int rc;
	
	if (config == NULL || config->name == NULL || config->ozone_uri == NULL) {
		SPDK_ERRLOG("Invalid configuration\n");
		return -EINVAL;
	}
	
	if (config->size_bytes == 0) {
		SPDK_ERRLOG("Size must be greater than 0\n");
		return -EINVAL;
	}
	
	ozone_bdev = calloc(1, sizeof(*ozone_bdev));
	if (ozone_bdev == NULL) {
		SPDK_ERRLOG("Failed to allocate ozone_bdev\n");
		return -ENOMEM;
	}
	
	ozone_bdev->ozone_uri = strdup(config->ozone_uri);
	if (ozone_bdev->ozone_uri == NULL) {
		free(ozone_bdev);
		return -ENOMEM;
	}
	
	ozone_bdev->size_bytes = config->size_bytes;
	ozone_bdev->block_size = config->block_size ? config->block_size : OZONE_DEFAULT_BLOCK_SIZE;
	ozone_bdev->chunk_size = config->chunk_size ? config->chunk_size : OZONE_DEFAULT_CHUNK_SIZE;
	
	/* Initialize SPDK bdev */
	ozone_bdev->bdev.name = strdup(config->name);
	if (ozone_bdev->bdev.name == NULL) {
		free(ozone_bdev->ozone_uri);
		free(ozone_bdev);
		return -ENOMEM;
	}
	
	ozone_bdev->bdev.product_name = "Ozone Block Device";
	ozone_bdev->bdev.module = &ozone_if;
	ozone_bdev->bdev.blocklen = ozone_bdev->block_size;
	ozone_bdev->bdev.blockcnt = ozone_bdev->size_bytes / ozone_bdev->block_size;
	ozone_bdev->bdev.ctxt = ozone_bdev;
	ozone_bdev->bdev.fn_table = &ozone_fn_table;
	
	/* TODO: Connect to Ozone via libhdfs */
	ozone_bdev->hdfs_handle = NULL;
	
	/* Register I/O device */
	spdk_io_device_register(ozone_bdev, bdev_ozone_create_cb, bdev_ozone_destroy_cb,
				sizeof(struct ozone_io_channel), config->name);
	
	/* Register bdev */
	rc = spdk_bdev_register(&ozone_bdev->bdev);
	if (rc != 0) {
		SPDK_ERRLOG("Failed to register bdev: %d\n", rc);
		spdk_io_device_unregister(ozone_bdev, NULL);
		free(ozone_bdev->ozone_uri);
		free((void *)ozone_bdev->bdev.name);
		free(ozone_bdev);
		return rc;
	}
	
	pthread_mutex_lock(&g_ozone_bdev_mutex);
	TAILQ_INSERT_TAIL(&g_ozone_bdev_head, ozone_bdev, link);
	pthread_mutex_unlock(&g_ozone_bdev_mutex);
	
	SPDK_NOTICELOG("Ozone bdev '%s' created: size=%lu, block_size=%u, chunk_size=%u\n",
		       config->name, ozone_bdev->size_bytes, ozone_bdev->block_size,
		       ozone_bdev->chunk_size);
	
	return 0;
}

/*
 * Delete an Ozone bdev
 */
int
ozone_bdev_delete(const char *name)
{
	struct ozone_bdev *ozone_bdev;
	
	if (name == NULL) {
		return -EINVAL;
	}
	
	pthread_mutex_lock(&g_ozone_bdev_mutex);
	TAILQ_FOREACH(ozone_bdev, &g_ozone_bdev_head, link) {
		if (strcmp(ozone_bdev->bdev.name, name) == 0) {
			pthread_mutex_unlock(&g_ozone_bdev_mutex);
			spdk_bdev_unregister(&ozone_bdev->bdev, NULL, NULL);
			return 0;
		}
	}
	pthread_mutex_unlock(&g_ozone_bdev_mutex);
	
	SPDK_ERRLOG("Ozone bdev '%s' not found\n", name);
	return -ENODEV;
}

/*
 * Get count of Ozone bdevs
 */
uint32_t
ozone_bdev_get_count(void)
{
	struct ozone_bdev *ozone_bdev;
	uint32_t count = 0;
	
	pthread_mutex_lock(&g_ozone_bdev_mutex);
	TAILQ_FOREACH(ozone_bdev, &g_ozone_bdev_head, link) {
		count++;
	}
	pthread_mutex_unlock(&g_ozone_bdev_mutex);
	
	return count;
}
