#!/bin/bash
# Simple MMIO via ami_tool (no AMI library needed). Run with sudo.
# Usage: sudo ./mmio_ami_tool.sh <BDF> [bar] [offset]
# Example: sudo ./mmio_ami_tool.sh 21:00.0 0 0x1000

set -e
BDF=${1:?Usage: $0 <BDF> [bar] [offset]}
BAR=${2:-0}
OFFSET=${3:-0x0}

echo "Read BAR${BAR} @ ${OFFSET}"
sudo ami_tool bar_rd -d "$BDF" -b "$BAR" -a "$OFFSET" -l 1

echo "Write 0xdeadbeef to BAR${BAR} @ ${OFFSET}"
sudo ami_tool bar_wr -d "$BDF" -b "$BAR" -a "$OFFSET" -i 0xdeadbeef -y

echo "Read back"
sudo ami_tool bar_rd -d "$BDF" -b "$BAR" -a "$OFFSET" -l 1
