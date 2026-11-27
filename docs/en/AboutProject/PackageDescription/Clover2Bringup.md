# clover2_bringup

The `clover2_bringup` package provides the launch files required to initialize the system's core components. It includes custom automation logic to streamline the configuration and startup process of the entire robot stack.

## Structure

The package is organized into specialized directories to separate hardware drivers, processing pipelines, and reusable configuration fragments:

```text
launch/
├── klever5.launch.xml       # Primary launch file for a complete Klever 5 system startup
├── description.launch.xml   # Publishes TFs based on the URDF description
├── sensing/                 # Device driver launch files
│   ├── camera.launch.xml
│   ├── 2d_lidar.launch.xml
│   └── thermal_camera.launch.xml
├── vision/                  # Image processing pipelines
│   ├── camera_pipeline.launch.xml
│   ├── feature_detector.launch.xml
│   └── optical_flow.launch.xml
└── snippets/                # Reusable launch file fragments
    ├── component_container.launch.xml
    ├── fcu_bridge.launch.xml
    ├── localization.launch.xml
    ├── indication.launch.xml
    └── display.launch.xml
```

## Main Launch File:  `klever5.launch.xml`

The `klever5.launch.xml` file serves as the central entry point for the simultaneous startup of all critical modules, including TF publishing, camera sensor suites, vision processing pipelines, the Flight Control Unit (FCU) bridge, localization, and diagnostic/indication tools.

### Configuration Arguments

You can customize the system startup by passing arguments to the launch file. Below are the primary configuration options available:

| Argument                                | Default | Description                             |
| --------------------------------------- | ------------ | --------------------------------------------------- |
| `main_camera.enable`                    | `true`       | Enables the main camera and its associated image processing pipeline  |
| `main_camera.feature_detector`          | `true`       | Enables the feature detector for the main camera  |
| `front_camera.enable`                   | `false`      | Enables the front-facing camera                           |
| `optical_flow.enable`                   | `true`       | Enables the optical flow processing                         |
| `fcu_bridge.fcu_conn`                   | `uart`       | Specifies the connection type to the Flight Control Unit (FCU)      |
| `localization.aruco_tracker.enable`     | `true`       | Enables ArUco marker tracking                      |
| `localization.map_server.enable`        | `true`       | Enables the map server                               |
| `localization.map_server.map_filename`  | `example-1.yaml`  |The filename of the map to be loaded                                 |
| `indication.led_strip`                  | `false`      | Enables the LED strip                                   |
| `indication.display`                    | `false`      | Enables the system display                                      |
| `additional_sensors.2d_lidar`           | `false`      | Enables the 2D LiDAR                                 |
| `additional_sensors.thermal_camera`     | `false`      | Enables the thermal camera                            |
| `sensing_pkg`                           | `clover2_bringup` | The package containing the sensor driver launch files |
| `root_ns`                               | —            | The root namespace (can be empty or start with `/`)    |
| `params_file`                           | `params/klever5.yaml` | Path to an external YAML file for additional parameters              |

To override default settings, append the desired arguments to your ros2 launch command.
For instance, to launch the system with the 2D LiDAR and the system display enabled, use the following command:

```
ros2 launch clover2_bringup klever5.launch.xml additional_sensors.2d_lidar:=true indication.display:=true
```

## Sensor Launch Group (sensing)

To ensure a unified and modular startup process, each sensor type has its own dedicated launch file located in the `launch/sensing/` directory, all of which adhere to a standardized argument structure.

### Camera Driver Arguments

Use these arguments to configure the camera driver launch files:

| Argument       | Required | Default          | Description                                                                                   |
| --------------- | ------------ | --------------------- | ---------------------------------------------------------------------------------------------- |
| `camera_id`     | Yes           |                       | Camera identifier (numeric index for libcamera or a device path)                                      |
| `camera_name`   | No         | `camera`              | Node name for the camera. Also used to locate the parameter file at `params/libcamera/$(camera_name).yaml`                     |
| `container_name`| No          | `camera_container`    | Name of the ROS 2 container used if loading the driver as a component                                 |
| `frame_id`      | No          | `$(camera_name)_link` | Coordinate frame identifier used in TF2                 |
| `ns`            | No         | —                     | Namespace for the camera node                                                                          |
| `params_file`   | No         | `params/klever5.yaml` | Path to an additional YAML parameters file                                                           |
| `use_composition` | No         | `false`               | If `true`, the driver is loaded as a component into  `$(container_name)`. If `false`, it runs as a standalone node  |

### 2D LiDAR Driver Arguments

Use these arguments to configure the 2D LiDAR launch files:

| Argument    | Required | Default          | Description                                  |
| ------------- | ------------ | ---------------------- | ------------------------------------------ |
| `lidar_id`    | No          | `/dev/rplidar`         | File path to the LiDAR's serial device |
| `lidar_name`  | No          | `lidar`                | Name of the LiDAR node                            |
| `ns`          | No         | —                      | Namespace for the LiDAR node                     |
| `params_file` | No         | `params/klever5.yaml`  | Path to an additional YAML parameters file     |

### Thermal Camera Driver Arguments

Use these arguments to configure the thermal camera launch files:

| Argument               | Required | Default          | Description                        |
| ----------------------- | ------------ | -------------------- | ---------------------------------- |
| `thermal_camera_device` | No         | `/dev/thermal_camera`| File path to the thermal camera device |
| `thermal_camera_name`   | No        | `thermal_camera`     | Name of the thermal camera node     |
| `ns`                    | No        | —                    | Namespace for the thermal camera node    |

### Overriding the Sensor Package

The main launch file,` klever5.launch.xml`, includes a `sensing_pkg` argument. This allows you to swap the default sensor drivers with custom ones without modifying the `clover2_bringup` source code.

To launch the system using a custom sensing package, use the following command:

```
ros2 launch clover2_bringup klever5.launch.xml sensing_pkg:=my_custom_sensing
```

The substituted package must mirror the directory structure and file names of the standard package to ensure the main launch file can locate them:

* `launch/sensing/2d_lidar.launch.xml`
*  `launch/sensing/camera.launch.xml`
*  `launch/sensing/thermal_camera.launch.xml`
