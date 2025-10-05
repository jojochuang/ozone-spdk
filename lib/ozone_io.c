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

/**
 * @file ozone_io.c
 * @brief Ozone I/O abstraction layer using libhdfs
 *
 * This file provides helper functions for interacting with Apache Ozone
 * through the libhdfs C API. It handles chunk-based storage, caching,
 * and async I/O operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <hdfs.h>

/**
 * Parse Ozone URI and extract connection info
 *
 * URI format: ofs://service-id/volume/bucket
 *
 * @param uri Ozone URI string
 * @param service_id Output buffer for service ID
 * @param volume Output buffer for volume name
 * @param bucket Output buffer for bucket name
 * @return 0 on success, -errno on failure
 */
int
ozone_parse_uri(const char *uri, char *service_id, char *volume, char *bucket)
{
	const char *prefix = "ofs://";
	char *uri_copy, *token, *saveptr;
	int ret = 0;
	
	if (uri == NULL || strncmp(uri, prefix, strlen(prefix)) != 0) {
		return -EINVAL;
	}
	
	uri_copy = strdup(uri + strlen(prefix));
	if (uri_copy == NULL) {
		return -ENOMEM;
	}
	
	/* Parse service_id */
	token = strtok_r(uri_copy, "/", &saveptr);
	if (token == NULL) {
		ret = -EINVAL;
		goto cleanup;
	}
	if (service_id != NULL) {
		strcpy(service_id, token);
	}
	
	/* Parse volume */
	token = strtok_r(NULL, "/", &saveptr);
	if (token == NULL) {
		ret = -EINVAL;
		goto cleanup;
	}
	if (volume != NULL) {
		strcpy(volume, token);
	}
	
	/* Parse bucket */
	token = strtok_r(NULL, "/", &saveptr);
	if (token == NULL) {
		ret = -EINVAL;
		goto cleanup;
	}
	if (bucket != NULL) {
		strcpy(bucket, token);
	}
	
cleanup:
	free(uri_copy);
	return ret;
}

/**
 * Connect to Ozone filesystem via libhdfs
 *
 * @param service_id Ozone service ID
 * @return hdfsFS handle on success, NULL on failure
 */
hdfsFS
ozone_connect(const char *service_id)
{
	hdfsFS fs;
	struct hdfsBuilder *builder;
	
	if (service_id == NULL) {
		return NULL;
	}
	
	builder = hdfsNewBuilder();
	if (builder == NULL) {
		return NULL;
	}
	
	hdfsBuilderSetNameNode(builder, service_id);
	hdfsBuilderSetNameNodePort(builder, 0); /* Use default */
	
	fs = hdfsBuilderConnect(builder);
	if (fs == NULL) {
		hdfsFreeBuilder(builder);
		return NULL;
	}
	
	return fs;
}

/**
 * Disconnect from Ozone filesystem
 *
 * @param fs Filesystem handle
 * @return 0 on success, -errno on failure
 */
int
ozone_disconnect(hdfsFS fs)
{
	int ret;
	
	if (fs == NULL) {
		return -EINVAL;
	}
	
	ret = hdfsDisconnect(fs);
	return (ret == 0) ? 0 : -EIO;
}

/**
 * Read a chunk from Ozone
 *
 * @param fs Filesystem handle
 * @param path Path to chunk file
 * @param offset Offset within chunk
 * @param buffer Output buffer
 * @param length Number of bytes to read
 * @return Number of bytes read on success, -errno on failure
 */
int
ozone_read_chunk(hdfsFS fs, const char *path, uint64_t offset,
		 void *buffer, size_t length)
{
	hdfsFile file;
	tSize bytes_read;
	
	if (fs == NULL || path == NULL || buffer == NULL) {
		return -EINVAL;
	}
	
	file = hdfsOpenFile(fs, path, O_RDONLY, 0, 0, 0);
	if (file == NULL) {
		return -EIO;
	}
	
	if (hdfsSeek(fs, file, offset) != 0) {
		hdfsCloseFile(fs, file);
		return -EIO;
	}
	
	bytes_read = hdfsRead(fs, file, buffer, length);
	hdfsCloseFile(fs, file);
	
	if (bytes_read < 0) {
		return -EIO;
	}
	
	return bytes_read;
}

/**
 * Write a chunk to Ozone
 *
 * @param fs Filesystem handle
 * @param path Path to chunk file
 * @param offset Offset within chunk
 * @param buffer Input buffer
 * @param length Number of bytes to write
 * @return Number of bytes written on success, -errno on failure
 */
int
ozone_write_chunk(hdfsFS fs, const char *path, uint64_t offset,
		  const void *buffer, size_t length)
{
	hdfsFile file;
	tSize bytes_written;
	int flags;
	
	if (fs == NULL || path == NULL || buffer == NULL) {
		return -EINVAL;
	}
	
	/* Check if file exists */
	if (hdfsExists(fs, path) == 0) {
		flags = O_WRONLY | O_APPEND;
	} else {
		flags = O_WRONLY;
	}
	
	file = hdfsOpenFile(fs, path, flags, 0, 0, 0);
	if (file == NULL) {
		return -EIO;
	}
	
	if (hdfsSeek(fs, file, offset) != 0) {
		hdfsCloseFile(fs, file);
		return -EIO;
	}
	
	bytes_written = hdfsWrite(fs, file, buffer, length);
	hdfsFlush(fs, file);
	hdfsCloseFile(fs, file);
	
	if (bytes_written < 0) {
		return -EIO;
	}
	
	return bytes_written;
}

/**
 * Delete a chunk from Ozone
 *
 * @param fs Filesystem handle
 * @param path Path to chunk file
 * @return 0 on success, -errno on failure
 */
int
ozone_delete_chunk(hdfsFS fs, const char *path)
{
	int ret;
	
	if (fs == NULL || path == NULL) {
		return -EINVAL;
	}
	
	ret = hdfsDelete(fs, path, 0);
	return (ret == 0) ? 0 : -EIO;
}

/**
 * Get chunk path for a given block offset
 *
 * @param volume Volume name
 * @param bucket Bucket name
 * @param bdev_name Bdev name
 * @param chunk_id Chunk ID
 * @param path Output buffer for path
 * @param path_size Size of output buffer
 * @return 0 on success, -errno on failure
 */
int
ozone_get_chunk_path(const char *volume, const char *bucket,
		     const char *bdev_name, uint64_t chunk_id,
		     char *path, size_t path_size)
{
	int ret;
	
	if (volume == NULL || bucket == NULL || bdev_name == NULL || path == NULL) {
		return -EINVAL;
	}
	
	ret = snprintf(path, path_size, "/%s/%s/%s/chunk_%lu",
		       volume, bucket, bdev_name, chunk_id);
	
	if (ret < 0 || (size_t)ret >= path_size) {
		return -ENAMETOOLONG;
	}
	
	return 0;
}
