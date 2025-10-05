#!/bin/bash
# Setup script for SPDK Ozone bdev
#
# Copyright (c) 2024 Wei-Chiu Chuang
# Licensed under the Apache License, Version 2.0

set -e

# Default values
BDEV_NAME="ozone0"
OZONE_URI=""
SIZE="1073741824"  # 1GB
BLOCK_SIZE="4096"
CHUNK_SIZE="4194304"  # 4MB
SPDK_RPC="${SPDK_ROOT}/scripts/rpc.py"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Setup SPDK Ozone block device

OPTIONS:
    -n NAME         Bdev name (default: ozone0)
    -u URI          Ozone URI (ofs://service/volume/bucket) [REQUIRED]
    -s SIZE         Size in bytes (default: 1073741824)
    -b BLOCKSIZE    Block size (default: 4096)
    -c CHUNKSIZE    Chunk size (default: 4194304)
    -r RPC_SCRIPT   Path to rpc.py (default: \$SPDK_ROOT/scripts/rpc.py)
    -h              Show this help message

EXAMPLES:
    # Create 10GB Ozone bdev
    $0 -n data0 -u "ofs://ozone/vol1/bucket1" -s 10737418240

    # Create with custom chunk size
    $0 -n db0 -u "ofs://ozone/vol1/db" -s 107374182400 -c 8388608

ENVIRONMENT:
    SPDK_ROOT       Path to SPDK installation

EOF
    exit 1
}

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_prerequisites() {
    log_info "Checking prerequisites..."
    
    if [ -z "$SPDK_ROOT" ]; then
        log_error "SPDK_ROOT environment variable not set"
        exit 1
    fi
    
    if [ ! -f "$SPDK_RPC" ]; then
        log_error "RPC script not found: $SPDK_RPC"
        exit 1
    fi
    
    if ! python3 "$SPDK_RPC" spdk_get_version &>/dev/null; then
        log_error "SPDK target not running or not responding"
        log_info "Start SPDK target with: sudo \$SPDK_ROOT/build/bin/spdk_tgt"
        exit 1
    fi
    
    log_info "Prerequisites OK"
}

create_bdev() {
    log_info "Creating Ozone bdev '$BDEV_NAME'..."
    
    # Check if bdev already exists
    if python3 "$SPDK_RPC" bdev_get_bdevs --name "$BDEV_NAME" &>/dev/null; then
        log_warn "Bdev '$BDEV_NAME' already exists"
        return 0
    fi
    
    # Create bdev
    python3 "$SPDK_RPC" ozone_bdev_create \
        --name "$BDEV_NAME" \
        --ozone-uri "$OZONE_URI" \
        --size "$SIZE" \
        --block-size "$BLOCK_SIZE" \
        --chunk-size "$CHUNK_SIZE"
    
    if [ $? -eq 0 ]; then
        log_info "Bdev created successfully"
    else
        log_error "Failed to create bdev"
        exit 1
    fi
}

show_bdev_info() {
    log_info "Bdev information:"
    python3 "$SPDK_RPC" bdev_get_bdevs --name "$BDEV_NAME" | python3 -m json.tool
}

# Parse command line arguments
while getopts "n:u:s:b:c:r:h" opt; do
    case $opt in
        n) BDEV_NAME="$OPTARG" ;;
        u) OZONE_URI="$OPTARG" ;;
        s) SIZE="$OPTARG" ;;
        b) BLOCK_SIZE="$OPTARG" ;;
        c) CHUNK_SIZE="$OPTARG" ;;
        r) SPDK_RPC="$OPTARG" ;;
        h) usage ;;
        *) usage ;;
    esac
done

# Validate required parameters
if [ -z "$OZONE_URI" ]; then
    log_error "Ozone URI is required (-u option)"
    usage
fi

# Main execution
log_info "SPDK Ozone Bdev Setup"
log_info "====================="
log_info "Name:       $BDEV_NAME"
log_info "URI:        $OZONE_URI"
log_info "Size:       $SIZE bytes"
log_info "Block Size: $BLOCK_SIZE bytes"
log_info "Chunk Size: $CHUNK_SIZE bytes"
log_info ""

check_prerequisites
create_bdev
show_bdev_info

log_info ""
log_info "Setup complete!"
log_info ""
log_info "Next steps:"
log_info "  1. Export as NBD:  python3 $SPDK_RPC nbd_start_disk $BDEV_NAME /dev/nbd0"
log_info "  2. Format device:  sudo mkfs.ext4 /dev/nbd0"
log_info "  3. Mount device:   sudo mount /dev/nbd0 /mnt"
log_info ""
log_info "To delete: python3 $SPDK_RPC ozone_bdev_delete --name $BDEV_NAME"
