# Documentation

Welcome to the SPDK Ozone Block Device Module documentation!

## 📚 Documentation Guide

Choose the guide that best fits your needs:

### 🚀 New Users

Start here if you're new to the project:

1. **[QUICKSTART.md](QUICKSTART.md)** - Get running in 5 minutes
   - Fastest path from zero to working setup
   - Step-by-step commands
   - Minimal explanation, maximum action

### 🔧 Installation & Setup

For detailed setup instructions:

2. **[BUILD.md](BUILD.md)** - Complete build and installation guide
   - System requirements and dependencies
   - SPDK installation
   - Hadoop/libhdfs setup
   - Build configuration options
   - Troubleshooting build issues

### 📖 Usage

Learn how to use the module:

3. **[USAGE.md](USAGE.md)** - Comprehensive usage guide
   - Starting SPDK target
   - Creating Ozone bdevs via JSON-RPC
   - Exporting via NBD, NVMe-oF, iSCSI
   - Testing with fio
   - Configuration files
   - Monitoring and debugging
   - Common use cases

### 🏗️ Architecture

Understand the design:

4. **[ARCHITECTURE.md](ARCHITECTURE.md)** - System architecture and design
   - Component overview
   - Data layout and chunk mapping
   - I/O path explanation
   - Thread model
   - Performance considerations
   - Error handling
   - Security

### 🤝 Contributing

Join the development:

5. **[../CONTRIBUTING.md](../CONTRIBUTING.md)** - Development guidelines
   - Setting up dev environment
   - Coding standards
   - Testing requirements
   - Pull request process
   - Code review guidelines

## 📊 Additional Resources

### Project Documentation
- **[../README.md](../README.md)** - Project overview and goals
- **[../STATUS.md](../STATUS.md)** - Current status and roadmap
- **[../LICENSE](../LICENSE)** - Apache 2.0 license

### Test Documentation
- **[../test/README.md](../test/README.md)** - Test suite guide

## 🔍 Quick Reference

### Essential Commands

**Build the module:**
```bash
export SPDK_ROOT=/path/to/spdk
export HADOOP_HOME=/usr/local/hadoop
make
```

**Start SPDK:**
```bash
sudo $SPDK_ROOT/build/bin/spdk_tgt -m 0x3
```

**Create Ozone bdev:**
```bash
$SPDK_ROOT/scripts/rpc.py ozone_bdev_create \
    --name ozone0 \
    --ozone-uri "ofs://service/volume/bucket" \
    --size 1073741824
```

**Export as NBD:**
```bash
$SPDK_ROOT/scripts/rpc.py nbd_start_disk ozone0 /dev/nbd0
```

### Environment Variables

| Variable | Description | Example |
|----------|-------------|---------|
| `SPDK_ROOT` | Path to SPDK installation | `/home/user/spdk` |
| `HADOOP_HOME` | Path to Hadoop installation | `/usr/local/hadoop` |
| `JAVA_HOME` | Path to Java JDK | `/usr/lib/jvm/java-11-openjdk` |
| `LD_LIBRARY_PATH` | Library search path | `$HADOOP_HOME/lib/native` |

### Key Concepts

- **Bdev**: SPDK block device - presents as a disk to applications
- **Ozone URI**: Format `ofs://service/volume/bucket` to specify storage location
- **Chunk**: Fixed-size (default 4MB) storage unit in Ozone
- **NBD**: Network Block Device - exposes bdev as `/dev/nbdX`
- **NVMe-oF**: NVMe over Fabrics - high-performance network block protocol
- **libhdfs**: C API for accessing Hadoop/Ozone storage

## 🎯 Common Tasks

### I want to...

**...get started quickly**
→ Read [QUICKSTART.md](QUICKSTART.md)

**...build from source**
→ Read [BUILD.md](BUILD.md)

**...understand the architecture**
→ Read [ARCHITECTURE.md](ARCHITECTURE.md)

**...use it in production**
→ Read [USAGE.md](USAGE.md)

**...contribute code**
→ Read [../CONTRIBUTING.md](../CONTRIBUTING.md)

**...troubleshoot an issue**
→ Check troubleshooting sections in [BUILD.md](BUILD.md) and [USAGE.md](USAGE.md)

**...run benchmarks**
→ See benchmarking section in [USAGE.md](USAGE.md)

**...integrate with Kubernetes**
→ See Kubernetes CSI section in [USAGE.md](USAGE.md)

## 📝 Documentation Standards

Our documentation follows these principles:

- **Clear**: Use simple language and concrete examples
- **Complete**: Cover all necessary information
- **Current**: Keep up-to-date with code changes
- **Concise**: Be thorough but avoid unnecessary details
- **Consistent**: Use consistent terminology and formatting

## 🔗 External Resources

### SPDK
- [SPDK Documentation](https://spdk.io/doc/)
- [SPDK Bdev Guide](https://spdk.io/doc/bdev.html)
- [SPDK GitHub](https://github.com/spdk/spdk)

### Apache Ozone
- [Ozone Documentation](https://ozone.apache.org/docs/)
- [Ozone GitHub](https://github.com/apache/ozone)
- [FSO Design](https://issues.apache.org/jira/browse/HDDS-2939)

### libhdfs
- [libhdfs API](https://hadoop.apache.org/docs/stable/hadoop-project-dist/hadoop-hdfs/LibHdfs.html)
- [Hadoop Documentation](https://hadoop.apache.org/docs/stable/)

## 🆘 Getting Help

1. **Check the docs** - Most questions are answered here
2. **Search issues** - Someone may have had the same problem
3. **Ask the community** - Open a GitHub issue or discussion
4. **Read the code** - The implementation is well-commented

## 📜 License

All documentation is licensed under Apache License 2.0, same as the code.

---

**Happy reading! If you find any documentation issues, please open a PR to improve it.** 📖✨
