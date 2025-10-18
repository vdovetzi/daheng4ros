#!/bin/bash
set -e

# Source ROS2 environment
source /opt/ros/jazzy/setup.bash
source /daheng_ws/install/setup.bash

# Set library path
export LD_LIBRARY_PATH=/daheng_ws/build/daheng4ros/:$LD_LIBRARY_PATH

# Initialize udev for device detection
if command -v udevd >/dev/null 2>&1; then
    if [ ! -d /run/udev ]; then
        mkdir -p /run/udev
    fi
    udevd --daemon 2>/dev/null || true
    udevadm trigger --action=add 2>/dev/null || true
    udevadm settle 2>/dev/null || true
fi

# Execute the command
exec "$@"