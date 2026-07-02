# clover2_bringup

This package contains all the launch files required to start the project, as well as custom actions for automated configuration setup.

## Sensing launch group

To standardize the startup method, device drivers have been moved to separate files in the folder `launch/sensing`. So, launch files have mandatory arguments.

### Camera driver launch template

| Argument        | Required | Default              | Description                                                                                             |
| --------------- | -------- | -------------------- | ------------------------------------------------------------------------------------------------------- |
| camera_id       | True     |                      | Camera identificator (for libcamera numeretic or dts path)                                              |
| camera_name     | False    | camera               | Cemera node name                                                                                        |
| container_name  | False    | camera_container     | ROS2 container name to load camera driver as component                                                  |
| frame_id        | False    | $(camera_name)\_link | TF2 vision frame id                                                                                     |
| ns              | False    | -                    | Camera ROS2 namespace                                                                                   |
| param_file      | False    | params/klever5.yaml  | Path to extra params file                                                                               |
| use_composition | False    | false                | If true - camera driver loading as componnet to container $(container_name). Else run as separated node |

### 2D Lidar driver launch template

| Argument    | Required | Default                                                    | Description                                                  |
| ----------- | -------- | ---------------------------------------------------------- | ------------------------------------------------------------ |
| lidar_id    | False    | /dev/rplidar0                                              | Path to lidar serial device                                  |
| lidar_name  | False    | lidar                                                      | Lidar node name                                              |
| ns          | False    | -                                                          | Lidar ROS2 namespace                                         |
| param_file  | False    | params/klever5.yaml                                        | Path to extra params file                                    |

### Override sensing package

The main `klever5.launch.xml` accepts `sensing_pkg` argument, which defines the package from which sensing launch files are loaded.
This allows you to substitute default 2D lidar and camera drivers with custom ones without modifying the original package.

| Argument      | Required | Default          | Description                                                      |
| ------------- | -------- | ---------------- | ---------------------------------------------------------------- |
| sensing_pkg   | False    | clover2_bringup  | Package name providing `launch/sensing/*.launch.xml` files       |

Usage example — run klever5 with custom sensing package:

```
ros2 launch clover2_bringup klever5.launch.xml sensing_pkg:=my_custom_sensing
```

The substituted package must provide the same launch file paths (e.g. `launch/sensing/2d_lidar.launch.xml`, `launch/sensing/camera.launch.xml`).
