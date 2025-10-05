# SPDK Block Device for Apache Ozone

This project implements a new [SPDK](https://spdk.io) block device module (`bdev_ozone`) that uses [Apache Ozone](https://ozone.apache.org/) as its backend storage.

The resulting block device can be exported through NVMe-oF, iSCSI, or NBD targets and consumed by higher-level systems such as **Kubernetes (via SPDK CSI)** and **OpenStack Cinder (via SPDK volume driver)**.

---

## 🎯 Goals

1. **Integrate Ozone with SPDK**
   - Implement a new SPDK bdev module (`bdev_ozone`) in C.
   - Access Ozone through **libhdfs** (C API) to perform reads and writes.
   - Store data as fixed-size chunks within an Ozone FSO bucket.

2. **Expose Ozone as a Block Device**
   - Present a local block device abstraction to SPDK.
   - Export volumes through NVMe-oF or iSCSI targets.
   - Allow consumption by SPDK Logical Volume Store (LVS).

3. **Support Cloud Orchestrators**
   - **Kubernetes** integration via [SPDK CSI](https://github.com/spdk/spdk-csi).
   - **OpenStack** integration via [SPDK Cinder Driver](https://docs.openstack.org/cinder/latest/configuration/block-storage/drivers/spdk-volume-driver.html).

4. **Ensure Data Consistency & Performance**
   - Use atomic rename on Ozone FSO buckets for crash-safe chunk updates.
   - Support read-modify-write for small random I/O.
   - Optionally add write-back caching and journaling for higher throughput.

---

## 🧩 Architecture Overview

```
+-------------------------------+
|  Kubernetes / OpenStack       |
|  (CSI / Cinder Driver)        |
+---------------+---------------+
                |
                v
+-------------------------------------------+
|  SPDK Target (NVMe-oF / iSCSI / NBD)     |
|  +-------------------------------------+  |
|  |  SPDK Logical Volume Store (LVS)   |  |
|  +-------------------------------------+  |
|  |  bdev_ozone (this project)         |  |
|  |  - Reads/Writes via libhdfs        |  |
|  |  - Chunked storage in Ozone FSO    |  |
|  +-------------------------------------+  |
+-------------------------------------------+
                |
                v
+-------------------------------------------+
|  Apache Ozone (OM + SCM + Datanodes)     |
|  - FSO-enabled bucket backend            |
|  - Data durability & scalability         |
+-------------------------------------------+
```

---

## 🚀 Quick Start

### Prerequisites

- SPDK v23.01 or later
- Apache Ozone cluster with FSO enabled
- libhdfs installed and configured
- GCC/Clang with C11 support

### Build

```bash
# Clone the repository
git clone https://github.com/jojochuang/ozone-spdk.git
cd ozone-spdk

# Set SPDK_ROOT environment variable
export SPDK_ROOT=/path/to/spdk

# Build the module
make

# Install (optional)
make install
```

### Usage

1. **Start SPDK application with ozone bdev module**:
```bash
./spdk_tgt -m 0x3
```

2. **Create an Ozone bdev via JSON-RPC**:
```bash
./scripts/rpc.py ozone_bdev_create \
    --name ozone0 \
    --size 1073741824 \
    --ozone-uri "ofs://ozone-service/volume/bucket" \
    --block-size 4096
```

3. **Use the bdev**:
```bash
# Export as NBD device
./scripts/rpc.py nbd_start_disk ozone0 /dev/nbd0

# Or export as NVMe-oF target
./scripts/rpc.py nvmf_create_transport -t TCP
./scripts/rpc.py nvmf_create_subsystem nqn.2023-01.io.spdk:ozone0
./scripts/rpc.py nvmf_subsystem_add_ns nqn.2023-01.io.spdk:ozone0 ozone0
```

---

## 🛠️ Development

### Project Structure

```
ozone-spdk/
├── module/           # SPDK bdev module implementation
│   ├── bdev_ozone.c
│   └── bdev_ozone.h
├── lib/              # Helper libraries
│   └── ozone_io.c    # Ozone I/O abstraction
├── include/          # Public headers
│   └── ozone_bdev.h
├── test/             # Unit and integration tests
├── scripts/          # Helper scripts
└── docs/             # Documentation
```

### Building for Development

```bash
# Build with debug symbols
make DEBUG=1

# Run tests
make test

# Clean build artifacts
make clean
```

---

## 🛠️ Planned Milestones

### ✅ Milestone 0 — Prototype
- [x] Create empty bdev module (`bdev_ozone` skeleton)
- [x] Link with `libhdfs` to verify Ozone connectivity
- [ ] Implement basic `READ`/`WRITE`/`UNMAP` path using chunk files
- [ ] Verify with `fio` via SPDK NBD/NVMe-oF

### Milestone 1 — Control Plane
- [ ] Add JSON-RPC methods: `ozone_bdev_create`, `ozone_bdev_delete`
- [ ] Integrate with SPDK Logical Volume Store (LVS)
- [ ] Validate CSI and Cinder end-to-end provisioning

### Milestone 2 — Optimization
- [ ] Implement write-back cache and async pipeline
- [ ] Support journaling, multi-threaded libhdfs I/O, and recovery
- [ ] Add CI tests and benchmarking scripts

---

## 💡 Long-Term Vision

A fully functional SPDK-Ozone integration that provides:
- NVMe-class performance on top of Ozone's scalable object storage
- Unified data layer for AI/ML workloads, cloud-native apps, and HPC
- A blueprint for bringing object storage into the SPDK ecosystem

---

## 📝 Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

---

## 📜 License

Apache License 2.0 (same as SPDK and Ozone)

See [LICENSE](LICENSE) for details.

---

## 👥 Authors

- **Wei-Chiu Chuang** – Initial design and development
- (Open for community contributions)

---

## 📚 Resources

- [SPDK Documentation](https://spdk.io/doc/)
- [Apache Ozone Documentation](https://ozone.apache.org/docs/)
- [SPDK Bdev Module Development Guide](https://spdk.io/doc/bdev.html)
- [libhdfs API Reference](https://hadoop.apache.org/docs/stable/hadoop-project-dist/hadoop-hdfs/LibHdfs.html)
