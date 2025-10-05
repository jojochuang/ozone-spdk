#!/usr/bin/env bash
# Script to build the Docker image for ozone-spdk development

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE_NAME="ozone-spdk-dev"
IMAGE_TAG="rocky9"

echo "Building Docker image: ${IMAGE_NAME}:${IMAGE_TAG}"
echo "This may take several minutes on first build..."

docker build \
    -t "${IMAGE_NAME}:${IMAGE_TAG}" \
    -f "${SCRIPT_DIR}/Dockerfile" \
    "${SCRIPT_DIR}"

echo ""
echo "Docker image built successfully: ${IMAGE_NAME}:${IMAGE_TAG}"
echo ""
echo "To run the container, use:"
echo "  ./docker-run.sh"
echo ""
echo "Or run manually with:"
echo "  docker run -it --rm -v \$(pwd):/workspace ${IMAGE_NAME}:${IMAGE_TAG}"
