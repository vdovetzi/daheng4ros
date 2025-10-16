from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.conditions import IfCondition
from ament_index_python import get_package_share_directory
import os


def generate_launch_description():
    config_file = os.path.join(
        get_package_share_directory('daheng4ros'),
        'params.yaml'
    )
    
    return LaunchDescription([
        DeclareLaunchArgument(
            'use_rqt',
            default_value='false',
        ),
        Node(
            package='rqt_reconfigure',
            executable='rqt_reconfigure',
            condition=IfCondition(LaunchConfiguration('use_rqt'))
        ),
        Node(
            package='daheng4ros',
            executable='image_publisher',
            parameters=[config_file]
        )
    ])