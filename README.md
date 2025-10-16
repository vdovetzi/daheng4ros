# daheng4ros

ROS2 driver for **Daheng Imaging** VEN-U3 Board Level Series cameras — simple image publisher with realtime parameter reconfiguration, basic image enhancements, and optional undistortion using camera calibration parameters.

---

## Table of contents


- [daheng4ros](#daheng4ros)
  - [Table of contents](#table-of-contents)
- [Features](#features)
- [Requirements](#requirements)
- [Parameters](#parameters)
- [Installation \& build](#installation--build)
- [Running](#running)
- [Contributing](#contributing)
- [License](#license)

---

# Features

- Use enhancements (`contrast`, `brightness`, `sharpen`).
- Publish `sensor_msgs/Image` on a configurable topic using `image_transport`.
- Runtime adjustable parameters: `exposure, gain, fps, brightness, contrast, sharpness, resolution`.
- Optional undistortion using `K` (camera matrix) and `D` (distortion coeffs).
- Simple C++ API wrapper for the camera (class `sdk_daheng::Camera`) with thread-based acquisition and functional-style callback.

---

# Requirements

- ROS 2 Jazzy
- C++17
- Daheng Imaging SDK installed on the host (GxIAPI)
- OpenCV (>=3.4 recommended) and `cv_bridge`.

> **Note:** The Daheng SDK (GxIAPI) is proprietary and must be installed separately according to Daheng's instructions. Ensure the required libraries are discoverable by your linker (LD_LIBRARY_PATH or system install).

---

---

# [Parameters](config/params.yaml)
```yaml
ros__parameters:
  camera_frame: "daheng_cam" # default
  camera_topic: "daheng/image" # default
  fps: 60
  exposure: 10000.0
  gain: 20.0
  width: 640
  height: 480
  brightness: 150 # -150~150
  contrast: 100 # -50~100
  sharpness: 0.5 # 0.1~5.0
  # uncomment the following with your parameters to publish undistorted image
  # K: [fx, 0, cx, 0, fy, cy, 0, 0, 1]
  # D: [k1, k2, p1, p2, k3]
```

---
# Installation & build

```bash
mkdir -p ~/ros2_ws/src && cd ~/ros2_ws/src
git clone https://github.com/vdovetzi/daheng4ros.git
cd ~/ros2_ws
rosdep install -y --from-path src/
colcon build --symlink-install
source ~/ros2_ws/install/setup.bash
```

---

# Running

```bash
ros2 launch daheng4ros image_publisher.launch.py
```

or directly:

```bash
ros2 run daheng4ros image_publisher --ros-args --params-file config/params.yaml
```

View images:

```bash
ros2 run rqt_image_view rqt_image_view
```
---
# Contributing

Pull requests and Issues are welcome!

---

# License

MIT License © 2025
