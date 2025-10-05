# Docker Development Environment Guide

This guide explains how to use the Docker-based development environment for ozone-spdk on systems where SPDK is not natively supported (e.g., macOS, Windows).

## Overview

The Docker environment provides a complete build environment based on Rocky Linux 9 with all necessary prerequisites for building SPDK and Ozone components.

## Prerequisites

- Docker Engine (version 20.10 or later)
- Docker Compose (optional, version 2.0 or later)
- At least 8GB of available disk space for the image
- At least 4GB of RAM allocated to Docker

## Getting Started

### Building the Docker Image

Build the Docker image using the provided script:

```bash
./docker-build.sh
```

This will create an image named `ozone-spdk-dev:rocky9` with all required dependencies.

### Running the Container

#### Option 1: Using the Shell Script (Recommended)

```bash
./docker-run.sh
```

This script:
- Automatically builds the image if it doesn't exist
- Mounts the current directory to `/workspace` in the container
- Runs in privileged mode (required for SPDK)
- Provides device access for SPDK operations

#### Option 2: Using Docker Compose

```bash
# Start the container in interactive mode
docker-compose run --rm ozone-spdk-dev

# Or with a custom command
docker-compose run --rm ozone-spdk-dev bash -c "make clean && make"
```

#### Option 3: Manual Docker Run

```bash
docker run -it --rm \
    --privileged \
    -v $(pwd):/workspace \
    -v /dev:/dev \
    ozone-spdk-dev:rocky9 \
    /bin/bash
```

## What's Included

The Docker image includes:

### Build Tools
- GCC 11.x (with g++)
- GNU Make
- CMake 3.20+
- Autoconf, Automake, Libtool
- pkg-config

### SPDK Prerequisites
- libaio-devel
- libiscsi-devel
- libuuid-devel
- libcurl-devel
- openssl-devel
- numactl-devel
- CUnit-devel
- jansson-devel
- ncurses-devel
- libbsd-devel
- fuse3-devel
- NASM assembler
- Meson and Ninja build systems
- Python 3 with pyelftools

### Java Development
- OpenJDK 17 (LTS)
- Apache Maven 3.9.x

### Other Tools
- Git
- Python 3 with pip
- Wget, curl
- Standard Unix utilities

## Usage Examples

### Building SPDK Inside the Container

```bash
# Clone SPDK repository
git clone https://github.com/spdk/spdk.git
cd spdk

# Initialize submodules
git submodule update --init

# Install additional dependencies (if needed)
sudo ./scripts/pkgdep.sh

# Configure and build
./configure
make -j$(nproc)

# Install SPDK
sudo make install
```

### Building ozone-spdk

```bash
cd /workspace

# Assuming SPDK is already built and installed
export SPDK_DIR=/path/to/spdk

# Build ozone-spdk
make all

# Or build specific targets
make hello_world_bdev_shared_combo
make nvme_shared
```

### Running Tests

```bash
cd /workspace

# Run the test suite (requires SPDK to be built)
sudo ./test_make.sh /path/to/spdk
```

## User Configuration

The container runs as a non-root user (`developer`) by default. This user has sudo privileges without password for convenience during development.

To switch to root if needed:
```bash
sudo -i
```

## Persistent Data

All files in the mounted workspace directory (`/workspace`) persist on your host system. Any files created outside this directory will be lost when the container exits.

## Troubleshooting

### Issue: Container fails to start with permission errors

**Solution**: Make sure Docker has the necessary permissions. On Linux, you may need to add your user to the docker group:
```bash
sudo usermod -aG docker $USER
```
Then log out and back in.

### Issue: Build fails with "permission denied" errors

**Solution**: Check that the mounted directory has appropriate permissions. The container runs as UID 1000 by default.

### Issue: SPDK requires hugepages

**Solution**: The container runs in privileged mode to support hugepage configuration. Inside the container, you can configure hugepages:
```bash
sudo /path/to/spdk/scripts/setup.sh
```

### Issue: Maven or Java not found

**Solution**: The environment variables should be set automatically. If not, add them to your shell:
```bash
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk
export MAVEN_HOME=/opt/maven
export PATH=$PATH:$MAVEN_HOME/bin
```

## Advanced Usage

### Customizing the Image

To add additional packages or modify the build environment, edit the `Dockerfile` and rebuild:

```bash
# Edit Dockerfile
vim Dockerfile

# Rebuild
./docker-build.sh
```

### Using a Different Base Image

To use a different Rocky Linux version or another RHEL-compatible distribution, modify the `FROM` line in the Dockerfile:

```dockerfile
FROM rockylinux:8
# or
FROM almalinux:9
```

### Sharing the Image

To share the built image with team members:

```bash
# Save the image
docker save ozone-spdk-dev:rocky9 | gzip > ozone-spdk-dev.tar.gz

# Load on another machine
docker load < ozone-spdk-dev.tar.gz
```

## Container Maintenance

### Updating the Image

To update the image with the latest packages:

```bash
# Rebuild without cache
docker build --no-cache -t ozone-spdk-dev:rocky9 -f Dockerfile .
```

### Cleaning Up

Remove the image when no longer needed:

```bash
docker rmi ozone-spdk-dev:rocky9
```

Remove all stopped containers:

```bash
docker container prune
```

## Performance Considerations

- The first build may take 10-15 minutes depending on your internet connection
- Subsequent builds use Docker's layer caching and are much faster
- Mount only necessary directories to improve I/O performance
- Consider using Docker volumes for build artifacts that don't need to persist on the host

## Support

For issues specific to the Docker environment, please open an issue on the GitHub repository with:
- Docker version (`docker --version`)
- Host operating system
- Error messages or logs
- Steps to reproduce the issue
