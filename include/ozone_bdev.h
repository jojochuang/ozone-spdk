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

#ifndef OZONE_BDEV_H
#define OZONE_BDEV_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ozone_bdev.h
 * @brief Public API for SPDK Ozone block device module
 */

/**
 * Configuration for creating an Ozone bdev
 */
struct ozone_bdev_config {
	const char *name;          /* Bdev name */
	const char *ozone_uri;     /* Ozone filesystem URI (ofs://service/volume/bucket) */
	uint64_t size_bytes;       /* Total size in bytes */
	uint32_t block_size;       /* Block size (typically 4096) */
	uint32_t chunk_size;       /* Chunk size for storage (default 4MB) */
};

/**
 * Create an Ozone-backed block device
 *
 * @param config Configuration parameters
 * @return 0 on success, negative errno on failure
 */
int ozone_bdev_create(const struct ozone_bdev_config *config);

/**
 * Delete an Ozone-backed block device
 *
 * @param name Name of the bdev to delete
 * @return 0 on success, negative errno on failure
 */
int ozone_bdev_delete(const char *name);

/**
 * Get the number of Ozone bdevs currently registered
 *
 * @return Number of active Ozone bdevs
 */
uint32_t ozone_bdev_get_count(void);

#ifdef __cplusplus
}
#endif

#endif /* OZONE_BDEV_H */
