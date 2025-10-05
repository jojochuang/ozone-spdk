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

#ifndef BDEV_OZONE_H
#define BDEV_OZONE_H

#include "spdk/stdinc.h"
#include "spdk/bdev.h"
#include "spdk/bdev_module.h"

/**
 * @file bdev_ozone.h
 * @brief Internal header for SPDK Ozone bdev module
 */

/* Default chunk size: 4MB */
#define OZONE_DEFAULT_CHUNK_SIZE (4 * 1024 * 1024)

/* Default block size: 4KB */
#define OZONE_DEFAULT_BLOCK_SIZE 4096

/**
 * Ozone bdev structure
 */
struct ozone_bdev {
	struct spdk_bdev bdev;           /* Base SPDK bdev */
	char *ozone_uri;                 /* Ozone URI (ofs://...) */
	uint64_t size_bytes;             /* Total size in bytes */
	uint32_t block_size;             /* Block size */
	uint32_t chunk_size;             /* Chunk size */
	void *hdfs_handle;               /* libhdfs filesystem handle */
	TAILQ_ENTRY(ozone_bdev) link;    /* Link in global list */
};

/**
 * Ozone I/O channel
 */
struct ozone_io_channel {
	void *hdfs_context;              /* Per-thread HDFS context */
};

/**
 * Initialize the Ozone bdev module
 */
int bdev_ozone_initialize(void);

/**
 * Finish/cleanup the Ozone bdev module
 */
void bdev_ozone_finish(void);

/**
 * Get the Ozone bdev module
 */
struct spdk_bdev_module *bdev_ozone_get_module(void);

#endif /* BDEV_OZONE_H */
