# Test Suite

This directory contains tests for the SPDK Ozone bdev module.

## Test Structure

```
test/
├── unit/           # Unit tests for individual components
├── integration/    # Integration tests with SPDK and Ozone
├── performance/    # Performance benchmarks
└── fixtures/       # Test data and mock objects
```

## Running Tests

### Prerequisites

- Built ozone-spdk module
- Running SPDK target (for integration tests)
- Running Ozone cluster (for integration tests)

### Unit Tests

```bash
make test
```

### Integration Tests

```bash
cd test/integration
./run_tests.sh
```

### Performance Tests

```bash
cd test/performance
./benchmark.sh
```

## Writing Tests

### Unit Test Template

```c
#include <check.h>
#include "ozone_bdev.h"

START_TEST(test_ozone_parse_uri)
{
    char service[256], volume[256], bucket[256];
    int ret;
    
    ret = ozone_parse_uri("ofs://ozone/vol1/bucket1",
                          service, volume, bucket);
    
    ck_assert_int_eq(ret, 0);
    ck_assert_str_eq(service, "ozone");
    ck_assert_str_eq(volume, "vol1");
    ck_assert_str_eq(bucket, "bucket1");
}
END_TEST

Suite *ozone_suite(void)
{
    Suite *s = suite_create("Ozone");
    TCase *tc = tcase_create("Core");
    
    tcase_add_test(tc, test_ozone_parse_uri);
    suite_add_tcase(s, tc);
    
    return s;
}
```

## Test Categories

### 1. Functional Tests
- Bdev creation and deletion
- Read/write operations
- Error handling
- URI parsing

### 2. Performance Tests
- Sequential read/write throughput
- Random I/O IOPS
- Latency measurements
- Scalability tests

### 3. Stress Tests
- Long-running operations
- High concurrency
- Resource exhaustion
- Failure recovery

### 4. Integration Tests
- NBD export
- NVMe-oF export
- SPDK LVS integration
- End-to-end workflows

## Test Tools

- **Check**: C unit testing framework
- **fio**: I/O benchmarking
- **SPDK bdevperf**: SPDK-specific benchmarking
- **Python**: Test automation scripts

## CI/CD

Tests are run automatically on:
- Pull requests
- Commits to main branch
- Nightly builds

See `.github/workflows/` for CI configuration.

## Troubleshooting

### Tests Fail to Connect to Ozone

Check:
1. Ozone cluster is running
2. Network connectivity
3. Authentication credentials
4. Environment variables (OZONE_CONF_DIR)

### SPDK Target Issues

```bash
# Check if target is running
ps aux | grep spdk_tgt

# View logs
tail -f /var/log/syslog | grep spdk

# Restart target
sudo pkill spdk_tgt
sudo $SPDK_ROOT/build/bin/spdk_tgt
```

## Contributing

When adding new features:
1. Write tests first (TDD approach)
2. Ensure all existing tests pass
3. Add integration tests if needed
4. Update this README if adding new test categories

See [CONTRIBUTING.md](../CONTRIBUTING.md) for more details.
