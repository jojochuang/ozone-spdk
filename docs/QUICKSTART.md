# Quick Start Guide

Get up and running with SPDK Ozone bdev in 5 minutes.

## Prerequisites

- Linux system (Ubuntu 20.04+ recommended)
- Root/sudo access
- Basic familiarity with command line

## Step 1: Install Dependencies

```bash
# Install system packages
sudo apt-get update
sudo apt-get install -y \
    gcc g++ make git \
    libssl-dev libaio-dev libnuma-dev \
    openjdk-11-jdk python3

# Set Java home
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
```

## Step 2: Install SPDK

```bash
# Clone and build SPDK (takes ~15 minutes)
git clone https://github.com/spdk/spdk.git
cd spdk
git checkout v23.01
git submodule update --init
sudo scripts/pkgdep.sh
./configure --with-shared
make -j$(nproc)

# Set SPDK_ROOT
export SPDK_ROOT=$(pwd)
cd ..
```

## Step 3: Install Hadoop/libhdfs

```bash
# Download Hadoop
wget https://downloads.apache.org/hadoop/common/hadoop-3.3.6/hadoop-3.3.6.tar.gz
tar -xzf hadoop-3.3.6.tar.gz
sudo mv hadoop-3.3.6 /usr/local/hadoop

# Set environment
export HADOOP_HOME=/usr/local/hadoop
export LD_LIBRARY_PATH=$HADOOP_HOME/lib/native:$LD_LIBRARY_PATH
```

## Step 4: Build Ozone Bdev Module

```bash
# Clone repository
git clone https://github.com/jojochuang/ozone-spdk.git
cd ozone-spdk

# Build
make

# Verify build
ls -lh libozone_bdev.*
```

## Step 5: Start SPDK Target

```bash
# Start SPDK target in background
cd $SPDK_ROOT
sudo ./build/bin/spdk_tgt -m 0x3 &

# Wait a moment for initialization
sleep 2

# Verify it's running
ps aux | grep spdk_tgt
```

## Step 6: Create Ozone Bdev

```bash
# Create a test bdev (Note: requires running Ozone cluster)
./scripts/rpc.py ozone_bdev_create \
    --name test0 \
    --ozone-uri "ofs://ozone-service/volume/bucket" \
    --size 1073741824 \
    --block-size 4096

# List bdevs
./scripts/rpc.py bdev_get_bdevs
```

## Step 7: Export and Use

### Option A: NBD (Simple)

```bash
# Export as network block device
./scripts/rpc.py nbd_start_disk test0 /dev/nbd0

# Format and mount
sudo mkfs.ext4 /dev/nbd0
sudo mkdir -p /mnt/ozone
sudo mount /dev/nbd0 /mnt/ozone

# Use it!
echo "Hello Ozone!" | sudo tee /mnt/ozone/test.txt
cat /mnt/ozone/test.txt

# Cleanup
sudo umount /mnt/ozone
./scripts/rpc.py nbd_stop_disk /dev/nbd0
```

### Option B: NVMe-oF (High Performance)

```bash
# Setup NVMe-oF target
./scripts/rpc.py nvmf_create_transport -t TCP
./scripts/rpc.py nvmf_create_subsystem nqn.2023-01.io.spdk:test0 --allow-any-host
./scripts/rpc.py nvmf_subsystem_add_ns nqn.2023-01.io.spdk:test0 test0
./scripts/rpc.py nvmf_subsystem_add_listener nqn.2023-01.io.spdk:test0 -t TCP -a 0.0.0.0 -s 4420

# On client machine
sudo modprobe nvme-tcp
sudo nvme connect -t tcp -n nqn.2023-01.io.spdk:test0 -a <server-ip> -s 4420
sudo nvme list
```

## Step 8: Run Benchmark

```bash
# Install fio
sudo apt-get install -y fio

# Run quick benchmark
sudo fio --name=test \
    --ioengine=libaio \
    --direct=1 \
    --rw=randread \
    --bs=4k \
    --numjobs=1 \
    --iodepth=32 \
    --runtime=30 \
    --time_based \
    --filename=/dev/nbd0
```

## Troubleshooting

### SPDK Target Won't Start

```bash
# Check for conflicting processes
sudo pkill spdk_tgt

# Check huge pages
cat /proc/meminfo | grep Huge

# Allocate huge pages if needed
sudo sh -c 'echo 1024 > /proc/sys/vm/nr_hugepages'
```

### Cannot Create Bdev

```bash
# Check SPDK logs
tail -f /var/log/syslog | grep spdk

# Verify Ozone cluster is reachable
hdfs dfs -ls ofs://ozone-service/

# Check environment variables
echo $HADOOP_HOME
echo $LD_LIBRARY_PATH
```

### NBD Device Not Available

```bash
# Load NBD kernel module
sudo modprobe nbd

# Check available NBD devices
ls -l /dev/nbd*
```

## Next Steps

- **Read full documentation**: [README.md](../README.md)
- **Build guide**: [BUILD.md](BUILD.md)
- **Usage guide**: [USAGE.md](USAGE.md)
- **Architecture**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **Contribute**: [CONTRIBUTING.md](../CONTRIBUTING.md)

## Getting Help

- **Issues**: https://github.com/jojochuang/ozone-spdk/issues
- **SPDK Community**: https://spdk.io/community/
- **Ozone Community**: https://ozone.apache.org/community/

---

**Congratulations! You now have a working SPDK Ozone block device.** 🎉
