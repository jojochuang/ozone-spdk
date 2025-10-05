#!/usr/bin/env bash
# Script to run the Docker container for ozone-spdk development

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE_NAME="ozone-spdk-dev"
IMAGE_TAG="rocky9"

# Check if image exists
if ! docker image inspect "${IMAGE_NAME}:${IMAGE_TAG}" &> /dev/null; then
    echo "Docker image ${IMAGE_NAME}:${IMAGE_TAG} not found."
    echo "Building the image first..."
    "${SCRIPT_DIR}/docker-build.sh"
fi

echo "Starting Docker container with ozone-spdk development environment..."
echo "Current directory will be mounted at /workspace in the container."
echo ""

# Run the container with:
# - Interactive terminal
# - Remove container on exit
# - Mount current directory to /workspace
# - Use privileged mode for SPDK hugepages and device access
# - Mount /dev for device access
docker run -it --rm \
    --privileged \
    -v "${SCRIPT_DIR}:/workspace" \
    -v /dev:/dev \
    "${IMAGE_NAME}:${IMAGE_TAG}" \
    /bin/bash

echo ""
echo "Container exited."
