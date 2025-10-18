FROM ros:jazzy-ros-base

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /daheng_ws/src/daheng4ros/

COPY . .

WORKDIR /daheng_ws

# Installing deps from package.xml via rosdep
RUN apt update && apt install -y \ 
    wget \
    udev \
    systemd \
    systemd-sysv \
    dbus \
    usbutils \
    unzip \
    && rosdep update \
    && rosdep install -y --from-paths src/ \
    && rm -rf /var/lib/apt/lists/*

# Installing Daheng Imaging SDK from their website
RUN wget https://gb.daheng-imaging.com/CN/Software/Cameras/Linux/Galaxy_Linux-x86_Gige-U3_32bits-64bits_2.4.2507.9231.zip \
    && unzip Galaxy_Linux-x86_Gige-U3_32bits-64bits_2.4.2507.9231.zip \
    && rm -rf Galaxy_Linux-x86_Gige-U3_32bits-64bits_2.4.2507.9231.zip \
    && cd Galaxy_Linux-x86_Gige-U3_32bits-64bits_2.4.2507.9231 \
    && chmod +x Galaxy_camera.run \
    && ./Galaxy_camera.run \
    && ldconfig

# Build the workspace
RUN /bin/bash -c "source /opt/ros/jazzy/setup.bash && colcon build"

# Set up environment for interactive use
RUN echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc \
    && echo "source /daheng_ws/install/setup.bash" >> ~/.bashrc \
    && echo "export LD_LIBRARY_PATH=/daheng_ws/build/daheng4ros/:$LD_LIBRARY_PATH" >> ~/.bashrc

# Create an entrypoint script for better initialization
COPY .docker/docker-entrypoint.sh /
RUN chmod +x /docker-entrypoint.sh

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["ros2", "run", "daheng4ros", "image_publisher", "--ros-args", "--params-file", "/daheng_ws/src/daheng4ros/config/params.yaml"]