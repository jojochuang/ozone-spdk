# ozone-spdk
SPDK block device module for Ozone

This directory is meant to demonstrate how to link an external application and bdev
module to the SPDK libraries. The makefiles contain six examples of linking against spdk
libraries. They cover linking an application both with and without a custom bdev. For each of
these categories, they also demonstrate linking against the spdk combined shared library,
individual shared libraries, and static libraries.

This directory also contains a convenient test script, test_make.sh, which automates making SPDK
and testing all six of these linker options. It takes a single argument, the path to an SPDK
repository and should be run as follows:

```bash
sudo ./test_make.sh /path/to/spdk
```

The application `hello_bdev` is a symlink and bdev module `passthru_external` have been copied from their namesakes
in the top level [SPDK github repository](https://github.com/spdk/spdk) and don't have any special
functionality.

## Pre-requisites

- SPDK: https://github.com/spdk/spdk
- libhdfs
- Apache Ozone

### Docker Build Environment

For development on systems where SPDK is not natively supported (e.g., macOS), we provide a Dockerized build environment based on Rocky Linux 9.

📖 **See [QUICKSTART-DOCKER.md](QUICKSTART-DOCKER.md) for a quick reference guide**

📚 **See [DOCKER.md](DOCKER.md) for comprehensive documentation**

#### Prerequisites
- Docker installed on your system
- Docker Compose (optional, for easier management)

#### Quick Start

1. **Build the Docker image:**
   ```bash
   ./docker-build.sh
   ```

2. **Run the development container:**
   ```bash
   ./docker-run.sh
   ```

   Or using Docker Compose:
   ```bash
   docker-compose run --rm ozone-spdk-dev
   ```

3. **Inside the container**, you can build the project:
   ```bash
   # First, clone and build SPDK (example)
   cd /workspace
   git clone https://github.com/spdk/spdk.git
   cd spdk
   git submodule update --init
   ./scripts/pkgdep.sh
   ./configure
   make -j$(nproc)
   
   # Then build ozone-spdk
   cd /workspace
   make
   ```

#### What's Included

The Docker environment includes:
- Rocky Linux 9 base system
- GCC, Make, CMake, and other build tools
- Git for version control
- SPDK prerequisites (libaio, DPDK dependencies, etc.)
- Java 17 (OpenJDK)
- Apache Maven 3.9.x
- Python 3 with required packages

#### Notes

- The container runs in privileged mode to support SPDK's hugepage and device access requirements
- Your current directory is mounted at `/workspace` in the container
- All changes made in `/workspace` persist on your host system

## Building ozone_bdev module

This project will build a SPDK block device module for Ozone called `ozone_bdev`.

### Linking with libhdfs

To link with libhdfs and configure runtime for libhdfs and Ozone, see the user documentation:
https://ozone.apache.org/docs/edge/interface/native-cpp.html
