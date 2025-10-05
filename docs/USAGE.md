# Usage Guide

This guide covers how to use the SPDK Ozone block device module.

## Prerequisites

- Built and installed `ozone-spdk` module (see [BUILD.md](BUILD.md))
- Running SPDK target application
- Apache Ozone cluster with FSO enabled
- Proper network connectivity and authentication

## Starting SPDK Target

### Basic Setup

```bash
cd $SPDK_ROOT

# Start SPDK target with default settings
sudo ./build/bin/spdk_tgt
```

### Advanced Configuration

```bash
# Specify CPU core mask and memory
sudo ./build/bin/spdk_tgt -m 0x3 -s 1024

# Enable logging for ozone module
sudo ./build/bin/spdk_tgt -m 0x3 --log-flag bdev_ozone

# Run in background
sudo ./build/bin/spdk_tgt -m 0x3 -d
```

**Parameters:**
- `-m` - CPU core mask (0x3 = cores 0 and 1)
- `-s` - Memory size in MB
- `-d` - Run as daemon
- `--log-flag` - Enable module-specific logging

## JSON-RPC Interface

SPDK uses JSON-RPC for management operations. You can use the Python RPC client or send raw JSON requests.

### Using Python RPC Client

```bash
cd $SPDK_ROOT/scripts

# List available RPC methods
./rpc.py --help
```

### Creating Ozone Bdev

```bash
# Create an Ozone bdev
./rpc.py ozone_bdev_create \
    --name ozone0 \
    --ozone-uri "ofs://ozone-service/volume/bucket" \
    --size 1073741824 \
    --block-size 4096 \
    --chunk-size 4194304
```

**Parameters:**
- `--name` - Bdev name (must be unique)
- `--ozone-uri` - Ozone URI (ofs://service/volume/bucket)
- `--size` - Total size in bytes
- `--block-size` - Block size (typically 4096)
- `--chunk-size` - Chunk size for storage (default 4MB)

**Example Response:**
```json
{
  "status": "success",
  "bdev": "ozone0"
}
```

### Listing Bdevs

```bash
# List all block devices
./rpc.py bdev_get_bdevs

# List specific bdev
./rpc.py bdev_get_bdevs --name ozone0
```

**Example Output:**
```json
[
  {
    "name": "ozone0",
    "product_name": "Ozone Block Device",
    "block_size": 4096,
    "num_blocks": 262144,
    "uuid": "12345678-1234-1234-1234-123456789abc",
    "driver_specific": {
      "ozone": {
        "ozone_uri": "ofs://ozone-service/volume/bucket",
        "chunk_size": 4194304
      }
    }
  }
]
```

### Deleting Ozone Bdev

```bash
./rpc.py ozone_bdev_delete --name ozone0
```

## Exporting Block Devices

### Via NBD (Network Block Device)

NBD provides a local block device that can be used like any disk.

```bash
# Start NBD device
./rpc.py nbd_start_disk ozone0 /dev/nbd0

# Verify device is available
ls -l /dev/nbd0
sudo fdisk -l /dev/nbd0

# Format and mount
sudo mkfs.ext4 /dev/nbd0
sudo mkdir -p /mnt/ozone
sudo mount /dev/nbd0 /mnt/ozone

# Use like any filesystem
echo "Hello Ozone!" | sudo tee /mnt/ozone/test.txt

# Stop NBD device when done
./rpc.py nbd_stop_disk /dev/nbd0
```

### Via NVMe-oF (NVMe over Fabrics)

NVMe-oF provides high-performance network block access.

#### 1. Configure NVMe-oF Transport

```bash
# Create TCP transport
./rpc.py nvmf_create_transport \
    --trtype TCP \
    --max-queue-depth 128 \
    --max-io-size 131072
```

#### 2. Create Subsystem

```bash
# Create NVMe subsystem
./rpc.py nvmf_create_subsystem \
    nqn.2023-01.io.spdk:ozone0 \
    --allow-any-host
```

#### 3. Add Namespace

```bash
# Add bdev as namespace
./rpc.py nvmf_subsystem_add_ns \
    nqn.2023-01.io.spdk:ozone0 \
    ozone0
```

#### 4. Add Listener

```bash
# Add TCP listener
./rpc.py nvmf_subsystem_add_listener \
    nqn.2023-01.io.spdk:ozone0 \
    -t TCP \
    -a 0.0.0.0 \
    -s 4420
```

#### 5. Connect from Client

```bash
# On client machine
sudo modprobe nvme-tcp

# Connect to target
sudo nvme connect \
    -t tcp \
    -n nqn.2023-01.io.spdk:ozone0 \
    -a <target-ip> \
    -s 4420

# List NVMe devices
sudo nvme list

# Use device (e.g., /dev/nvme1n1)
sudo mkfs.ext4 /dev/nvme1n1
sudo mount /dev/nvme1n1 /mnt/ozone
```

### Via iSCSI

```bash
# Create iSCSI portal group
./rpc.py iscsi_create_portal_group \
    1 \
    0.0.0.0:3260

# Create initiator group (allow all)
./rpc.py iscsi_create_initiator_group \
    1 \
    ANY \
    ANY

# Create target
./rpc.py iscsi_create_target_node \
    iqn.2023-01.io.spdk:ozone0 \
    "ozone0:0" \
    "1:1" \
    64 \
    --disable-chap

# Connect from initiator
sudo iscsiadm -m discovery -t st -p <target-ip>:3260
sudo iscsiadm -m node --login
```

## Testing with FIO

FIO is a flexible I/O benchmarking tool.

### Basic Read Test

```bash
sudo fio --name=read-test \
    --ioengine=libaio \
    --direct=1 \
    --rw=read \
    --bs=4k \
    --numjobs=1 \
    --iodepth=32 \
    --runtime=60 \
    --time_based \
    --group_reporting \
    --filename=/dev/nbd0
```

### Basic Write Test

```bash
sudo fio --name=write-test \
    --ioengine=libaio \
    --direct=1 \
    --rw=write \
    --bs=4k \
    --numjobs=1 \
    --iodepth=32 \
    --runtime=60 \
    --time_based \
    --group_reporting \
    --filename=/dev/nbd0
```

### Random Read/Write Mix

```bash
sudo fio --name=randmix-test \
    --ioengine=libaio \
    --direct=1 \
    --rw=randrw \
    --rwmixread=70 \
    --bs=4k \
    --numjobs=4 \
    --iodepth=64 \
    --runtime=120 \
    --time_based \
    --group_reporting \
    --filename=/dev/nbd0
```

### Using SPDK bdevperf

SPDK includes its own benchmarking tool:

```bash
cd $SPDK_ROOT

# Create config file
cat > /tmp/ozone.conf << EOF
[Ozone]
  Name ozone0
  OzoneUri ofs://ozone-service/volume/bucket
  Size 1073741824
  BlockSize 4096
EOF

# Run benchmark
sudo ./build/examples/bdevperf \
    -c /tmp/ozone.conf \
    -q 128 \
    -o 4096 \
    -w randread \
    -t 60
```

## Configuration File

You can define bdevs in a configuration file:

```json
{
  "subsystems": [
    {
      "subsystem": "bdev",
      "config": [
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
      ]
    }
  ]
}
```

Load configuration at startup:

```bash
sudo ./build/bin/spdk_tgt -c /path/to/config.json
```

## Monitoring and Debugging

### View Logs

```bash
# SPDK logs to syslog by default
tail -f /var/log/syslog | grep spdk

# Or specify log file
sudo ./build/bin/spdk_tgt -L /tmp/spdk.log
tail -f /tmp/spdk.log
```

### Get Statistics

```bash
# Get bdev statistics
./rpc.py bdev_get_iostat --name ozone0

# Reset statistics
./rpc.py bdev_reset_iostat
```

### Debug Mode

```bash
# Run with debug logging
sudo ./build/bin/spdk_tgt \
    -m 0x3 \
    -L /tmp/spdk.log \
    --log-flag bdev_ozone \
    --log-flag bdev \
    --log-level DEBUG
```

## Common Use Cases

### 1. Simple File Storage

```bash
# Create bdev
./rpc.py ozone_bdev_create --name data0 \
    --ozone-uri "ofs://ozone/vol1/data" \
    --size 10737418240  # 10GB

# Export as NBD
./rpc.py nbd_start_disk data0 /dev/nbd0

# Format and mount
sudo mkfs.ext4 /dev/nbd0
sudo mount /dev/nbd0 /mnt/data

# Use normally
cp /home/user/files/* /mnt/data/
```

### 2. Database Storage

```bash
# Create bdev for database
./rpc.py ozone_bdev_create --name db0 \
    --ozone-uri "ofs://ozone/vol1/database" \
    --size 107374182400  # 100GB

# Export via NVMe-oF for better performance
# (See NVMe-oF section above)

# Configure database to use NVMe device
# e.g., PostgreSQL data directory on /dev/nvme1n1
```

### 3. Kubernetes CSI

```bash
# Deploy SPDK CSI driver
kubectl apply -f spdk-csi-deployment.yaml

# Create storage class
kubectl apply -f ozone-storage-class.yaml

# Use in pod
kubectl apply -f pod-with-ozone-volume.yaml
```

## Cleanup

```bash
# Stop NBD devices
./rpc.py nbd_stop_disk /dev/nbd0

# Delete bdevs
./rpc.py ozone_bdev_delete --name ozone0

# Stop SPDK target
sudo pkill spdk_tgt
```

## Troubleshooting

### Connection Issues

```bash
# Test Ozone connectivity
hdfs dfs -ls ofs://ozone-service/volume/bucket/

# Check SPDK logs
tail -f /var/log/syslog | grep "bdev_ozone"
```

### Performance Issues

```bash
# Check I/O statistics
./rpc.py bdev_get_iostat

# Verify chunk size is appropriate
# Larger chunks = better throughput
# Smaller chunks = better for random I/O
```

### Permission Issues

```bash
# Ensure proper Ozone permissions
ozone sh volume addacl /volume -a user:spdk:rwlc

# Check Kerberos authentication if enabled
klist
kinit spdk-user@REALM
```

## Next Steps

- See [ARCHITECTURE.md](ARCHITECTURE.md) for design details
- See [CONTRIBUTING.md](../CONTRIBUTING.md) for development
- Join the community for support and discussions
