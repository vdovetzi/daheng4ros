"""
Launch file for daheng4ros package.

This launch file starts the Daheng camera image publisher node and optionally
launches rqt_reconfigure for dynamic parameter tuning.
"""

import os

from ament_index_python import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration

from launch_ros.actions import Node


def generate_launch_description():
    """
    Generate launch description for daheng4ros nodes.

    Returns
    -------
    LaunchDescription
        Launch description containing:
        - DeclareLaunchArgument for 'use_rqt' parameter
        - Conditional rqt_reconfigure node
        - Daheng camera image publisher node with parameters from params.yaml

    Notes
    -----
    The 'use_rqt' argument controls whether rqt_reconfigure is launched.
    Default value is 'false'. Set to 'true' to enable dynamic parameter tuning.

    """
    config_file = os.path.join(
        get_package_share_directory('daheng4ros'),
        'params.yaml'
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'use_rqt',
            default_value='false',
            description='Whether to launch rqt_reconfigure'
        ),
        Node(
            package='rqt_reconfigure',
            executable='rqt_reconfigure',
            condition=IfCondition(LaunchConfiguration('use_rqt')),
            name='rqt_reconfigure',
            description='Dynamic reconfigure GUI for parameter tuning'
        ),
        Node(
            package='daheng4ros',
            executable='image_publisher',
            parameters=[config_file],
            name='daheng_camera_publisher',
            description='Daheng camera image publisher node'
        )
    ])
