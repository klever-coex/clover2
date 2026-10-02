# How to Create a Marker Map

<br>

## Using YAML Format

You can describe a map using the YAML format. This format helps you store map settings in a user-friendly way and add new settings without changing the file structure.

> New to YAML? Check out our [quick guide to YAML](../UsefulInformation/YAML).

```yaml
name: simulation          # Map name (by default, the file name)
default_size: 0.36        # Default marker size in meters 

markers:
  - id: 0                 # Unique marker ID from the ArUco dictionary (Mandatory)
    # size: 0.15          # Size of a specific marker. If this parameter is not specified, `default_size` is used
    pose:
      x: 0.0              # X-axis position (m)
      y: 0.0              # Y-axis position (m)
      # z: 0.0            # Z-axis position (m), default is 0
    rot:
      yaw: -1.57079632    # Rotation around the vertical axis (radians), optional
      # roll: 0.0         # Rotation around the front-to-back axis (radians), optional
      # pitch: 0.0        # Rotation around the side-to-side axis, optional
```

### Marker Fields

| Field | Mandatory | Default value | Description |
|---|---|---|---|
| `id` | Yes | - | Unique marker ID from the ArUco dictionary |
| `size` | No | `default_size` | Marker side length in meters. If this parameter is not set, `default_size` is used. If neither parameter is set, the program will report an error. |
| `pose.x` | No | `0.0` | X-axis position (in meters) |
| `pose.y` | No | `0.0` | Y-axis position (in meters) |
| `pose.z` | No | `0.0` | Z-axis position (in meters) |
| `rot.roll` | No | `0.0` | Roll rotation (in radians) |
| `rot.pitch` | No | `0.0` | Pitch rotation (in radians) |
| `rot.yaw` | No | `0.0` | Yaw rotation (in radians) |
| `quat.x/y/z/w` | No | `(0,0,0,1)` | Alternative way to set rotation using quaternions |
| `frame_id` | No | `"{frame_id}_aruco_{id}"` | Custom name for this marker's coordinate system |

You have two different ways to tell the computer how a marker is rotated: you can use  `rot` (RPY) or `quat` (quaternions). You must pick one or the other—do not try to use both at the same time.

### Example: A Base Map Using `default_size`

```yaml
name: my_room
frame_id: map
version: 0
default_size: 0.30

markers:
  - id: 12
    pose:
      x: 1.0
      y: 0.5
  - id: 13
    pose:
      x: 1.0
      y: 1.5
```

In this example, both markers use `size=0.30`. This value is automatically pulled from the `default_size` parameter.

### Example: Custom Size and Rotation

```yaml
markers:
  - id: 0
    size: 0.25
    pose:
      x: 0.0
      y: 0.0
      z: 0.5
    rot:
      yaw: 1.57
      pitch: 0.1
```

In this example, the marker with `id=0` has `size=0.25`. Orientation is specified with the yaw and pitch parameters.

### Example: Using Quaternions Instead of RPY

```yaml
markers:
  - id: 42
    pose:
      x: 2.0
      y: 3.0
    quat:
      x: 0.0
      y: 0.0
      z: 0.707
      w: 0.707
```

In this example, instead of using Roll, Pitch, and Yaw (RPY), the marker’s orientation is defined using the quat (quaternion) method.

## TXT Format (Optional)

```{caution}
We highly recommend using YAML. It offers more configuration options and provides a much more flexible structure.
```

You can also save your map in a plain text format using the `.txt` extension:

```
id size x y [z yaw pitch roll]
```

In this format, each line represents a single marker, with values separated by spaces. Any line starting with `#` is treated as a comment and will be ignored by the program.

```
# id size x y z yaw pitch roll
0 0.36 0.0 0.0 0.0 -1.57 0.0 0.0
1 0.36 0.0 -1.0 0.0 -1.57 0.0 0.0
2 0.36 0.0 -2.0 0.0 -1.57 0.0 0.0
```

The `z`, `yaw`, `pitch`, and `roll` fields are optional. If you leave them out, the program will default their values to `0.0`.

## How the Map Is Loaded

When the system starts, it follows these steps to process your map:

1. The `clover2_map` searches your map file at the location specified in the `map_path` parameter.
2. The program checks the file extension to decide whether to read it as `.yaml`/`.yml` or `.txt`.
3. For each marker found, the program creates a `Geometry_msgs/PoseWithCovariance` and a static TF frame.
4. By default, the TF frame is named using this template: `{frame_id}_aruco_{id}` (for example, `map_aruco_0`).
5. If you have assigned a specific `frame_id` to a marker, the program will use your custom name instead of the default one.
6. The program shares the map via the  `~/get_map` service and publishes updates to the `~/map_update` topic when the map is updated.

## Creating a Map for a Room

To build an accurate map for your environment, follow these steps:

1. Print your ArUco markers. Use the `4X4_1000` dictionary from the ArUco library.  
2. Place the markers on the ceiling or walls along the intended flight path.
3. Measure the position of each marker with a tape measure. All measurements must be taken relative to your chosen reference point.
4. Record the `id`, `pose.x/y/z`, and `rot.yaw` parameters in a YAML file.
5. In your YAML file, set the  `default_size` to the actual side length of the marker in meters.

```{caution}
The size refers specifically to the side length of the black square surrounding the ArUco pattern.
```

## Map and Camera Testing

### Testing the Topics

Before running full flight tests, you must verify that the camera is sending a debug image and correctly detecting the markers.

Open the `/main_camera/feat_detector/output/debug` topic in your browser and point the camera at your markers. The detected markers should be highlighted in the video feed.

You can also confirm detection via the quadcopter’s terminal by monitoring  `/main_camera/feat_detector/output/markers` topic. When the camera sees a marker, data messages should begin appearing.

```bash
ros2 topic echo /main_camera/feat_detector/output/markers
```

### Verifying Position Holding Over a Field of Markers

Once you have confirmed that the topics are working correctly, you need to ensure the quadcopter can actually use the markers to maintain its position.

1. Place the quadcopter directly on an area covered with ArUco markers.
2. Switch the flight controller to `Stabilized` mode.
3. Perform a manual takeoff.
4. After takeoff, hold the throttle stick at approximately 50%.
5. Switch the flight mode to `Position`.
6. Make sure the quadcopter is hovering autonomously and maintaining its position relative to the marker field.
