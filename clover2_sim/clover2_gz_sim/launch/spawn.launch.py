from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def _urdf_basename(model_str: str) -> str:
    if model_str.endswith(".urdf"):
        return model_str
    legacy = {
        "x500": "x500.urdf",
        "x500_mono_cam_down": "x500.urdf",
        "clover5": "clover5.urdf",
    }
    return legacy.get(model_str, "x500.urdf")


def launch_setup(context, *args, **kwargs):
    world = LaunchConfiguration("world")
    name = LaunchConfiguration("name")
    model = LaunchConfiguration("model")

    urdf_name = _urdf_basename(model.perform(context))
    urdf_file = PathJoinSubstitution(
        [FindPackageShare("clover2_description"), "urdf", urdf_name]
    )

    spawn_cmd = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=[
            "-world",
            world,
            "-name",
            name,
            "-x",
            "0",
            "-y",
            "0",
            "-z",
            "0.3",
            "-topic",
            "/robot_description",
        ],
    )
    return [spawn_cmd]


def generate_launch_description():
    world_declare = DeclareLaunchArgument(
        "world",
        description="Gazebo world.",
    )

    model_declare = DeclareLaunchArgument(
        "model",
        default_value="x500",
        description=("Name of model."),
    )

    name_declare = DeclareLaunchArgument(
        "name",
        default_value="x500",
        description="Model name in simulation.",
    )

    return LaunchDescription(
        [
            world_declare,
            model_declare,
            name_declare,
            OpaqueFunction(function=launch_setup),
        ]
    )
