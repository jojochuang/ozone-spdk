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

## Building ozone_bdev module

This project will build a SPDK block device module for Ozone called `ozone_bdev`.

### Linking with libhdfs

To link with libhdfs and configure runtime for libhdfs and Ozone, see the user documentation:
https://ozone.apache.org/docs/edge/interface/native-cpp.html
