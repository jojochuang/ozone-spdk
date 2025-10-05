# Build and Installation Guide

This guide covers building and installing the SPDK Ozone block device module.

## Prerequisites

### System Requirements

- **OS**: Linux (Ubuntu 20.04+ or CentOS 8+ recommended)
- **Memory**: Minimum 4GB RAM
- **Storage**: At least 10GB free space for build artifacts
- **CPU**: x86_64 or ARM64 architecture

### Software Dependencies

#### Required

1. **SPDK v23.01 or later**
   - Build tools and libraries
   - Development headers

2. **Apache Ozone**
   - Running cluster with FSO enabled
   - Network connectivity to Ozone service

3. **libhdfs**
   - Hadoop native libraries
   - Java JDK (OpenJDK 11 recommended)

4. **Build Tools**
   - GCC 7.0+ or Clang 6.0+
   - GNU Make
   - Git

#### Optional

- **fio** - For I/O benchmarking
- **Docker** - For containerized testing
- **Valgrind** - For memory debugging

## Building SPDK

If you don't have SPDK installed, follow these steps:

### 1. Install System Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y \
    gcc g++ make \
    libssl-dev libaio-dev libnuma-dev \
    uuid-dev libiscsi-dev python3 python3-pip \
    pkg-config libcunit1-dev
```

**CentOS/RHEL:**
```bash
sudo yum install -y \
    gcc gcc-c++ make \
    openssl-devel libaio-devel numactl-devel \
    libuuid-devel libiscsi-devel python3 \
    CUnit-devel
```

### 2. Clone and Build SPDK

```bash
# Clone SPDK repository
git clone https://github.com/spdk/spdk.git
cd spdk

# Checkout stable version
git checkout v23.01
git submodule update --init

# Install dependencies
sudo scripts/pkgdep.sh

# Configure
./configure --with-shared

# Build (this may take 10-20 minutes)
make -j$(nproc)

# Set environment variable
export SPDK_ROOT=$(pwd)
```

## Installing Hadoop and libhdfs

### 1. Install Java

**Ubuntu/Debian:**
```bash
sudo apt-get install -y openjdk-11-jdk
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
```

**CentOS/RHEL:**
```bash
sudo yum install -y java-11-openjdk-devel
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk
```

### 2. Download Hadoop

```bash
# Download Hadoop 3.3.6 (or latest stable)
cd /tmp
wget https://downloads.apache.org/hadoop/common/hadoop-3.3.6/hadoop-3.3.6.tar.gz
tar -xzf hadoop-3.3.6.tar.gz
sudo mv hadoop-3.3.6 /usr/local/hadoop

# Set environment variables
export HADOOP_HOME=/usr/local/hadoop
export LD_LIBRARY_PATH=$HADOOP_HOME/lib/native:$LD_LIBRARY_PATH
```

### 3. Build Native Libraries (Optional)

If pre-built libhdfs is not available:

```bash
cd $HADOOP_HOME

# Install Maven
sudo apt-get install -y maven  # Ubuntu
# or
sudo yum install -y maven      # CentOS

# Build with native libraries
mvn package -Pdist,native -DskipTests -Dtar

# Verify libhdfs.so exists
ls -l $HADOOP_HOME/lib/native/libhdfs.so
```

## Building Ozone Bdev Module

### 1. Clone Repository

```bash
git clone https://github.com/jojochuang/ozone-spdk.git
cd ozone-spdk
```

### 2. Set Environment Variables

```bash
export SPDK_ROOT=/path/to/spdk
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
export HADOOP_HOME=/usr/local/hadoop
```

### 3. Build Module

**Standard build:**
```bash
make
```

**Debug build:**
```bash
make DEBUG=1
```

**Verbose build:**
```bash
make V=1
```

### 4. Verify Build

```bash
# Check for build artifacts
ls -lh libozone_bdev.a libozone_bdev.so

# Should see:
# libozone_bdev.a  - Static library
# libozone_bdev.so - Shared library
```

## Installation

### Install to SPDK Directory

```bash
make install
```

This will:
- Copy libraries to `$SPDK_ROOT/lib/`
- Copy headers to `$SPDK_ROOT/include/ozone/`

### Install to System

```bash
sudo make install PREFIX=/usr/local
```

This will:
- Copy libraries to `/usr/local/lib/`
- Copy headers to `/usr/local/include/ozone/`
- Update library cache

## Verification

### 1. Check Library Dependencies

```bash
ldd libozone_bdev.so
```

Should show:
- libhdfs.so
- libspdk libraries
- Standard system libraries

### 2. Verify SPDK Integration

```bash
# Check if SPDK can find the module
cd $SPDK_ROOT
./build/bin/spdk_tgt --help
```

## Troubleshooting

### Build Failures

**Error: Cannot find SPDK headers**
```
Solution: Set SPDK_ROOT environment variable
export SPDK_ROOT=/path/to/spdk
```

**Error: libhdfs.so not found**
```
Solution: Set HADOOP_HOME and LD_LIBRARY_PATH
export HADOOP_HOME=/usr/local/hadoop
export LD_LIBRARY_PATH=$HADOOP_HOME/lib/native:$LD_LIBRARY_PATH
```

**Error: jni.h not found**
```
Solution: Set JAVA_HOME correctly
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
```

### Runtime Errors

**Error: Cannot load libozone_bdev.so**
```bash
# Add to library path
export LD_LIBRARY_PATH=$SPDK_ROOT/lib:$LD_LIBRARY_PATH

# Or update system library cache
sudo ldconfig
```

**Error: HDFS connection failed**
```
Check:
1. Ozone cluster is running
2. Network connectivity to Ozone service
3. HDFS configuration files in $HADOOP_HOME/etc/hadoop/
4. Correct permissions and authentication
```

## Development Build

For active development:

```bash
# Enable all warnings and debug info
make DEBUG=1 V=1

# Clean before rebuild
make clean
make DEBUG=1

# Run with SPDK in debug mode
cd $SPDK_ROOT
./build/bin/spdk_tgt -m 0x3 -s 1024 --log-flag bdev_ozone
```

## Build Configuration

### Makefile Variables

- `SPDK_ROOT` - Path to SPDK installation (required)
- `JAVA_HOME` - Path to Java JDK (required)
- `HADOOP_HOME` - Path to Hadoop installation (required)
- `DEBUG=1` - Enable debug build
- `V=1` - Verbose build output
- `CC` - C compiler (default: gcc)
- `CFLAGS` - Additional compiler flags
- `LDFLAGS` - Additional linker flags

### Example Custom Build

```bash
make \
    SPDK_ROOT=/opt/spdk \
    JAVA_HOME=/usr/lib/jvm/java-11-openjdk \
    HADOOP_HOME=/usr/local/hadoop \
    DEBUG=1 \
    CFLAGS="-march=native" \
    -j8
```

## Next Steps

- See [USAGE.md](USAGE.md) for running and testing
- See [CONTRIBUTING.md](../CONTRIBUTING.md) for development guidelines
- See [ARCHITECTURE.md](ARCHITECTURE.md) for design details
