# Contributing to SPDK Ozone Block Device Module

Thank you for your interest in contributing to the SPDK Ozone bdev module! This document provides guidelines for contributing to the project.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR-USERNAME/ozone-spdk.git
   cd ozone-spdk
   ```
3. **Add upstream remote**:
   ```bash
   git remote add upstream https://github.com/jojochuang/ozone-spdk.git
   ```

## Development Environment

### Prerequisites

- **SPDK**: v23.01 or later ([installation guide](https://spdk.io/doc/getting_started.html))
- **Apache Ozone**: Latest stable release with FSO enabled
- **libhdfs**: Hadoop native libraries with HDFS C API
- **Build tools**: GCC/Clang, Make, Git
- **Optional**: Docker for containerized testing

### Setting Up

1. Install SPDK:
   ```bash
   git clone https://github.com/spdk/spdk.git
   cd spdk
   git checkout v23.01
   git submodule update --init
   ./scripts/pkgdep.sh
   ./configure
   make
   ```

2. Install Hadoop/Ozone dependencies:
   ```bash
   # Install Java
   sudo apt-get install openjdk-11-jdk

   # Download and extract Hadoop
   wget https://downloads.apache.org/hadoop/common/hadoop-3.3.6/hadoop-3.3.6.tar.gz
   tar -xzf hadoop-3.3.6.tar.gz
   export HADOOP_HOME=/path/to/hadoop-3.3.6
   
   # Build native libraries with libhdfs
   cd $HADOOP_HOME
   mvn package -Pdist,native -DskipTests -Dtar
   ```

3. Build the Ozone bdev module:
   ```bash
   cd ozone-spdk
   export SPDK_ROOT=/path/to/spdk
   make DEBUG=1
   ```

## Development Workflow

### Creating a Branch

Always create a feature branch for your work:

```bash
git checkout -b feature/my-new-feature
```

Use descriptive branch names:
- `feature/` - New features
- `bugfix/` - Bug fixes
- `docs/` - Documentation updates
- `test/` - Test improvements

### Making Changes

1. **Write clean, maintainable code**
   - Follow the existing code style
   - Keep functions small and focused
   - Add comments for complex logic
   - Use meaningful variable names

2. **Follow SPDK coding conventions**
   - Use tabs for indentation (width: 8)
   - Keep lines under 120 characters
   - Place opening braces on the same line
   - Use `snake_case` for functions and variables

3. **Include proper error handling**
   - Check return values
   - Use appropriate errno values
   - Log errors with SPDK_ERRLOG
   - Clean up resources on error paths

### Testing

1. **Build your changes**:
   ```bash
   make clean
   make DEBUG=1
   ```

2. **Run existing tests**:
   ```bash
   make test
   ```

3. **Add new tests** for your changes in `test/` directory

4. **Manual testing** with SPDK tools:
   ```bash
   # Start SPDK target
   sudo ./spdk_tgt -m 0x3
   
   # Test your changes with JSON-RPC
   ./scripts/rpc.py ozone_bdev_create --name test0 --size 1073741824 --ozone-uri "ofs://..."
   ```

### Committing Changes

1. **Make atomic commits**
   - One logical change per commit
   - Keep commits small and focused

2. **Write good commit messages**:
   ```
   module: Short summary (50 chars or less)
   
   More detailed explanation if needed. Wrap at 72 characters.
   Explain what and why, not how.
   
   - Bullet points are okay
   - Use present tense: "Add feature" not "Added feature"
   
   Fixes: #123
   ```

3. **Sign your commits**:
   ```bash
   git commit -s -m "Your commit message"
   ```

### Submitting Pull Requests

1. **Update your branch** with latest upstream:
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Push to your fork**:
   ```bash
   git push origin feature/my-new-feature
   ```

3. **Create a Pull Request** on GitHub:
   - Use a clear, descriptive title
   - Reference related issues
   - Describe what you changed and why
   - Include testing details

4. **Respond to review feedback**:
   - Address all comments
   - Push updates to your branch
   - Be respectful and constructive

## Code Style Guide

### C Code Style

- **Indentation**: Tabs (width 8)
- **Line length**: Maximum 120 characters
- **Braces**: K&R style (opening brace on same line)
- **Naming**:
  - Functions: `lowercase_with_underscores`
  - Variables: `lowercase_with_underscores`
  - Constants: `UPPERCASE_WITH_UNDERSCORES`
  - Structs: `struct lowercase_with_underscores`

Example:
```c
static int
bdev_ozone_read(struct ozone_bdev *bdev, uint64_t offset,
		void *buffer, size_t length)
{
	int ret;
	
	if (bdev == NULL || buffer == NULL) {
		SPDK_ERRLOG("Invalid parameters\n");
		return -EINVAL;
	}
	
	ret = hdfs_read(bdev->fs, offset, buffer, length);
	if (ret < 0) {
		SPDK_ERRLOG("Read failed: %d\n", ret);
		return ret;
	}
	
	return 0;
}
```

### Comments

- Use `/* */` for multi-line comments
- Use `//` sparingly for single-line comments
- Document all public APIs with function headers
- Explain complex algorithms or business logic

### Error Handling

- Return negative errno values on error
- Return 0 on success
- Use appropriate error codes (EINVAL, EIO, ENOMEM, etc.)
- Always clean up resources on error paths

## Documentation

- Update README.md for user-facing changes
- Add/update comments for code changes
- Update docs/ for architecture or design changes
- Include examples where helpful

## Review Process

1. Maintainers will review your PR
2. Address feedback and push updates
3. Once approved, maintainers will merge
4. Keep discussion respectful and constructive

## License

By contributing, you agree that your contributions will be licensed under the Apache License 2.0.

## Questions?

- Open an issue for bugs or feature requests
- Ask questions in pull request comments
- Reach out to maintainers for guidance

## Code of Conduct

Be respectful, inclusive, and professional in all interactions. We follow the Apache Software Foundation Code of Conduct.

---

**Thank you for contributing to the SPDK Ozone bdev module!**
