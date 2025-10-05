# Dockerfile for ozone-spdk build environment
# Based on Rocky Linux 9 with SPDK and Ozone prerequisites

FROM rockylinux:9

# Set environment variables
ENV LANG=en_US.UTF-8 \
    LC_ALL=en_US.UTF-8 \
    JAVA_HOME=/usr/lib/jvm/java-17-openjdk \
    MAVEN_HOME=/opt/maven \
    PATH=$PATH:$MAVEN_HOME/bin

# Install EPEL repository for additional packages
RUN dnf install -y epel-release && \
    dnf update -y

# Install build essentials and development tools
RUN dnf groupinstall -y "Development Tools" && \
    dnf install -y --allowerasing \
    git \
    gcc \
    gcc-c++ \
    make \
    cmake \
    autoconf \
    automake \
    libtool \
    pkg-config \
    wget \
    curl \
    which \
    findutils \
    diffutils \
    patch \
    python3 \
    python3-pip \
    sudo

# Install SPDK prerequisites
# Based on SPDK requirements: https://spdk.io/doc/getting_started.html
RUN dnf install -y dnf-plugins-core && \
    dnf config-manager --set-enabled crb && \
    dnf install -y \
    libaio-devel \
    libiscsi-devel \
    libuuid-devel \
    libcurl-devel \
    openssl-devel \
    libnl3-devel \
    numactl-devel \
    CUnit-devel \
    json-c-devel \
    ncurses-devel \
    libbsd-devel \
    libarchive-devel \
    fuse3-devel \
    nasm

# Install Python packages needed for SPDK
RUN pip3 install --upgrade pip && \
    pip3 install pyelftools meson ninja

# Install Java 17 (OpenJDK)
RUN dnf install -y java-17-openjdk java-17-openjdk-devel

# Install Maven 3.9.x
RUN MAVEN_VERSION=3.9.6 && \
    wget https://archive.apache.org/dist/maven/maven-3/${MAVEN_VERSION}/binaries/apache-maven-${MAVEN_VERSION}-bin.tar.gz -O /tmp/maven.tar.gz && \
    mkdir -p /opt/maven && \
    tar -xzf /tmp/maven.tar.gz -C /opt/maven --strip-components=1 && \
    rm /tmp/maven.tar.gz && \
    ln -s /opt/maven/bin/mvn /usr/local/bin/mvn

# Create a non-root user for development
RUN useradd -m -s /bin/bash developer && \
    echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Set working directory
WORKDIR /workspace

# Change ownership to developer user
RUN chown -R developer:developer /workspace

# Switch to developer user
USER developer

# Verify installations
RUN echo "Build environment ready!" && \
    gcc --version && \
    make --version && \
    cmake --version && \
    git --version && \
    java -version && \
    mvn --version

# Default command
CMD ["/bin/bash"]
