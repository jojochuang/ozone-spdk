# Architecture Documentation

## Overview

The SPDK Ozone block device module (`bdev_ozone`) integrates Apache Ozone object storage with the SPDK block device framework. It presents Ozone storage as a standard block device that can be consumed by various SPDK targets and applications.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    User Applications                         │
│              (Kubernetes, OpenStack, etc.)                   │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   SPDK Target Layer                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   NVMe-oF    │  │    iSCSI     │  │     NBD      │      │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘      │
└─────────┼──────────────────┼──────────────────┼─────────────┘
          │                  │                  │
          └──────────────────┴──────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   SPDK Bdev Layer                            │
│  ┌──────────────────────────────────────────────────────┐   │
│  │          Logical Volume Store (LVS)                  │   │
│  └────────────────────┬─────────────────────────────────┘   │
│                       │                                      │
│  ┌────────────────────▼─────────────────────────────────┐   │
│  │              bdev_ozone (this module)                │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │  • I/O Request Queue                           │  │   │
│  │  │  • Chunk Management                            │  │   │
│  │  │  • Cache Layer (optional)                      │  │   │
│  │  └────────────────────────────────────────────────┘  │   │
│  └────────────────────┬─────────────────────────────────┘   │
└───────────────────────┼─────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│                  libhdfs Interface                           │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  • hdfsFS connection management                      │   │
│  │  • hdfsFile operations (open/read/write/close)       │   │
│  │  • Path resolution                                   │   │
│  └──────────────────────────────────────────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                   Apache Ozone                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │     OM       │  │     SCM      │  │  Datanodes   │      │
│  │  (Metadata)  │  │  (Containers)│  │   (Data)     │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────────────────────────────────────────────┘
```

## Components

### 1. SPDK Bdev Module (`bdev_ozone`)

The core component that implements the SPDK bdev interface.

**Key Responsibilities:**
- Register with SPDK bdev framework
- Handle I/O requests (READ, WRITE, UNMAP, FLUSH)
- Manage I/O channels for per-thread context
- Interface with libhdfs

**Files:**
- `module/bdev_ozone.c` - Main module implementation
- `module/bdev_ozone.h` - Internal module header
- `include/ozone_bdev.h` - Public API header

### 2. Ozone I/O Layer (`ozone_io`)

Helper library for Ozone-specific operations.

**Key Responsibilities:**
- URI parsing and connection management
- Chunk file operations
- Path generation
- Error handling

**Files:**
- `lib/ozone_io.c` - I/O abstraction layer

### 3. libhdfs Integration

HDFS C API for accessing Ozone storage.

**Key Operations:**
- `hdfsConnect()` - Connect to Ozone service
- `hdfsOpenFile()` - Open chunk files
- `hdfsRead()`/`hdfsWrite()` - I/O operations
- `hdfsSeek()` - Position within file
- `hdfsFlush()` - Ensure data persistence

## Data Layout

### Block to Chunk Mapping

Ozone storage is organized into fixed-size chunks:

```
Block Device (Logical View):
┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│  0  │  1  │  2  │  3  │  4  │  5  │  6  │  7  │  ... (blocks)
└─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
  4KB   4KB   4KB   4KB   4KB   4KB   4KB   4KB

Ozone Storage (Physical View):
┌─────────────────────────────────────────────┐
│           Chunk 0 (4MB)                     │  /volume/bucket/bdev/chunk_0
├─────────────────────────────────────────────┤
│           Chunk 1 (4MB)                     │  /volume/bucket/bdev/chunk_1
├─────────────────────────────────────────────┤
│           Chunk 2 (4MB)                     │  /volume/bucket/bdev/chunk_2
└─────────────────────────────────────────────┘
```

**Mapping Formula:**
```
chunk_id = block_offset / (chunk_size / block_size)
offset_in_chunk = (block_offset % (chunk_size / block_size)) * block_size
```

### Directory Structure in Ozone

```
/{volume}/{bucket}/
├── {bdev_name}/
│   ├── metadata.json         # Bdev metadata
│   ├── chunk_0               # First 4MB chunk
│   ├── chunk_1               # Second 4MB chunk
│   ├── chunk_2               # Third 4MB chunk
│   └── ...
```

## I/O Path

### Read Operation

1. **Request arrives** at `bdev_ozone_submit_request()`
2. **Calculate chunk ID** and offset within chunk
3. **Read chunk** via `ozone_read_chunk()`
   - Open HDFS file
   - Seek to offset
   - Read data into buffer
   - Close file
4. **Complete I/O** with `spdk_bdev_io_complete()`

### Write Operation

1. **Request arrives** at `bdev_ozone_submit_request()`
2. **Calculate chunk ID** and offset within chunk
3. **Write chunk** via `ozone_write_chunk()`
   - Open/create HDFS file
   - Seek to offset
   - Write data from buffer
   - Flush to ensure durability
   - Close file
4. **Complete I/O** with `spdk_bdev_io_complete()`

### UNMAP Operation

1. **Request arrives** for UNMAP/TRIM
2. **Identify affected chunks**
3. **Zero out or delete** chunk files
4. **Complete I/O**

## Thread Model

### I/O Channels

Each SPDK thread has its own I/O channel:

```
┌──────────────────────────────────────────────────────────┐
│                  SPDK Thread 0                            │
│  ┌────────────────────────────────────────────────────┐  │
│  │  I/O Channel 0                                     │  │
│  │  • hdfs context                                    │  │
│  │  • Request queue                                   │  │
│  └────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────┐
│                  SPDK Thread 1                            │
│  ┌────────────────────────────────────────────────────┐  │
│  │  I/O Channel 1                                     │  │
│  │  • hdfs context                                    │  │
│  │  • Request queue                                   │  │
│  └────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
```

**Benefits:**
- No locking between threads
- Per-thread HDFS contexts
- Scalable I/O processing

## Configuration

### Bdev Creation Parameters

```json
{
  "method": "ozone_bdev_create",
  "params": {
    "name": "ozone0",
    "ozone_uri": "ofs://ozone-service/volume/bucket",
    "size": 1073741824,
    "block_size": 4096,
    "chunk_size": 4194304
  }
}
```

### Ozone URI Format

```
ofs://{service-id}/{volume}/{bucket}
```

**Examples:**
- `ofs://ozone1/vol1/bucket1`
- `ofs://om-service-id/production/data`

## Performance Considerations

### Current Implementation (Milestone 0)

- **Synchronous I/O**: Each request blocks until completion
- **No caching**: Direct HDFS operations
- **No pipelining**: Operations are sequential

### Future Optimizations (Milestone 2)

1. **Write-back cache**
   - Cache hot chunks in memory
   - Batch writes for better throughput
   - Reduce latency for repeated access

2. **Async I/O**
   - Non-blocking HDFS operations
   - Parallel chunk operations
   - Better thread utilization

3. **Journaling**
   - Transaction log for consistency
   - Fast recovery after crashes
   - Atomic multi-chunk updates

4. **Read-ahead**
   - Prefetch sequential chunks
   - Reduce latency for streaming reads

## Error Handling

### Failure Scenarios

1. **HDFS connection lost**
   - Retry with exponential backoff
   - Fail I/O if retries exhausted
   - Log error and notify upper layers

2. **Chunk read/write failure**
   - Return appropriate errno
   - Mark I/O as failed
   - Preserve data consistency

3. **Out of space**
   - Return ENOSPC
   - Allow reads to continue
   - Prevent further writes

### Recovery Mechanisms

1. **Connection recovery**
   - Automatic reconnection on disconnect
   - Maintain connection pool

2. **Transaction log** (future)
   - Replay incomplete operations
   - Ensure consistency after crash

## Security

### Authentication

- Use Kerberos for Ozone authentication
- Configure via HDFS configuration files
- Pass credentials through libhdfs

### Authorization

- Ozone ACLs control access
- Verify permissions before operations
- Handle permission errors gracefully

## Testing

### Unit Tests

- Component-level testing
- Mock HDFS operations
- Verify error handling

### Integration Tests

- End-to-end with real Ozone cluster
- Performance benchmarking with fio
- Stress testing and fault injection

### Test Tools

- `fio` - I/O benchmarking
- `SPDK bdevperf` - SPDK-specific testing
- Custom test scripts

## References

- [SPDK Bdev Programming Guide](https://spdk.io/doc/bdev.html)
- [Apache Ozone Documentation](https://ozone.apache.org/docs/)
- [libhdfs API Reference](https://hadoop.apache.org/docs/stable/hadoop-project-dist/hadoop-hdfs/LibHdfs.html)
