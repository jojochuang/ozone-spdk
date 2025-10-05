# Quick Start Guide for Docker Environment

This is a quick reference guide for developers using the Docker-based build environment for ozone-spdk.

## TL;DR

```bash
# Build the Docker image
./docker-build.sh

# Start the container
./docker-run.sh

# Inside the container, you're ready to build!
```

## One-Time Setup

### 1. Build the Docker Image (First time only)

```bash
./docker-build.sh
```

This takes about 10-15 minutes the first time. Subsequent builds are cached and much faster.

## Daily Workflow

### 2. Start Your Development Container

```bash
./docker-run.sh
```

This drops you into a bash shell inside the container with:
- All build tools ready
- Your current directory mounted at `/workspace`
- Privileged mode for SPDK device access

### 3. Build SPDK (if not already done)

```bash
# Clone SPDK (first time only)
cd /workspace
git clone https://github.com/spdk/spdk.git
cd spdk
git submodule update --init

# Install system dependencies
sudo ./scripts/pkgdep.sh

# Configure SPDK
./configure --with-shared

# Build SPDK (use all CPU cores)
make -j$(nproc)

# Optionally install SPDK system-wide
sudo make install
```

### 4. Build ozone-spdk

```bash
cd /workspace

# Set SPDK paths if SPDK is not installed system-wide
export SPDK_DIR=/workspace/spdk
export SPDK_HEADER_DIR=${SPDK_DIR}/include
export SPDK_LIB_DIR=${SPDK_DIR}/build/lib

# Build ozone-spdk
make all

# Or build specific components
make hello_world_bdev_shared_combo
make nvme_shared
make accel_shared
```

## Common Commands

### Building

```bash
# Clean build artifacts
make clean

# Build everything
make all

# Build static libraries
make static
```

### Testing

```bash
# Run tests (requires SPDK to be built)
sudo ./test_make.sh /path/to/spdk
```

## Troubleshooting

### "Permission denied" errors

```bash
# Make sure you're in the container's workspace
cd /workspace

# Check file ownership
ls -la

# Fix ownership if needed (from host machine)
# sudo chown -R $USER:$USER .
```

### Missing dependencies

```bash
# Update package list
sudo dnf update -y

# Install additional packages as needed
sudo dnf install -y package-name
```

### Hugepages for SPDK

```bash
# Setup hugepages (inside container)
cd /path/to/spdk
sudo ./scripts/setup.sh

# Verify hugepages
grep Huge /proc/meminfo
```

## Tips & Tricks

### Keep the Container Running

If you want to keep the container running between sessions:

```bash
# Start in detached mode
docker run -d --name ozone-spdk-dev --privileged \
    -v $(pwd):/workspace -v /dev:/dev \
    ozone-spdk-dev:rocky9 sleep infinity

# Attach to it anytime
docker exec -it ozone-spdk-dev bash

# Stop it when done
docker stop ozone-spdk-dev
docker rm ozone-spdk-dev
```

### Rebuild After Code Changes

The container sees all changes in your mounted directory immediately:

```bash
# Edit files on host with your favorite editor
# Then in the container, just rebuild:
make clean && make
```

### Share Build Artifacts

All files in `/workspace` are on your host machine, so build artifacts persist:

```bash
# In container
make all

# On host
ls -la ./hello_world/hello_bdev  # The binary is there!
```

## macOS Users

The Docker environment is specifically designed for macOS where SPDK cannot run natively. All development and testing happens inside the Linux container.

## Next Steps

For more detailed information, see:
- [DOCKER.md](DOCKER.md) - Comprehensive Docker environment guide
- [README.md](README.md) - General project documentation
- [SPDK Documentation](https://spdk.io/doc/getting_started.html) - SPDK getting started guide

## Getting Help

If you encounter issues:

1. Check the [DOCKER.md](DOCKER.md) troubleshooting section
2. Verify Docker is running: `docker --version`
3. Check container logs: `docker logs <container-id>`
4. Open an issue on GitHub with details

Happy building! 🚀
