# Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause-Clear

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

from launch_ros.substitutions import FindPackageShare
from launch.substitutions import PathJoinSubstitution


def generate_launch_description():
    """Launch file to bring up qrb ros amr service node standalone."""

    qrb_ros_vla_state_node = Node(
        package='qrb_ros_vla_state',
        executable='qrb_ros_vla_state',
        output={'stderr': 'screen', 'stdout': 'log'},
        emulate_tty=False
    )

    return LaunchDescription([qrb_ros_vla_state_node])
