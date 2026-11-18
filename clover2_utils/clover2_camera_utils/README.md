# Camera calibration

Start the calibration node:

```bash
ros2 run clover2_camera_utils camera_calibration
```

The action goal specifies a self-describing JSON calibration target profile on
the host running `camera_calibration`. Each profile declares its target `type`.
Example profiles are installed in
`share/clover2_camera_utils/config/boards`:

```json
{
  "type": "chessboard",
  "rows": 6,
  "columns": 9,
  "square_size_m": 0.0225
}
```

For ChArUco, use a profile with the number of squares, marker size, and ArUco
dictionary:

```json
{
  "type": "charuco",
  "squares_rows": 7,
  "squares_columns": 10,
  "square_size_m": 0.03,
  "marker_size_m": 0.022,
  "dictionary": "DICT_4X4_50"
}
```

Start calibration with feedback:

```bash
ros2 action send_goal \
  --feedback \
  /camera_calibration/calibrate \
  clover2_camera_utils_msgs/action/CalibrateCamera \
  "{
    camera_namespace: '/main_camera/camera',
    calibration_method: 'fisheye',
    target_config_path: '/absolute/path/to/clover2/clover2_utils/clover2_camera_utils/config/boards/charuco_4x6_65mm_47_667mm_A3.json'
  }"
```

After successful completion, the node sends the calculated parameters to
`/camera_namespace/set_camera_info` (`sensor_msgs/srv/SetCameraInfo`). If the
camera driver service is unavailable or rejects the request, calibration still
succeeds: the calculated parameters are returned in the action result and
printed to the log.

## Finish calibration

```bash
ros2 service call \
  /camera_calibration/finish \
  clover2_camera_utils_msgs/srv/FinishCalibration \
  "{goal_id: 'GOAL_ID_FROM_FEEDBACK'}"
```

The goal ID is included in the action feedback. Copy it into the `finish`
service request to complete the corresponding calibration session. Multiple
sessions can run concurrently.

## C++ client

`calibration_client` wraps the calibration action and finish service. It saves
the session UUID from action feedback; completion remains explicit:

```cpp
#include <clover2_camera_utils/calibration_client.hpp>

clover2_camera_utils::calibration_client client(node);

clover2_camera_utils::calibration_client::calibrate_camera::Goal goal;
goal.camera_namespace = "/main_camera/camera";
goal.calibration_method = "fisheye";
goal.target_config_path = "/absolute/path/to/target.json";

client.start(goal, [](auto /* goal_handle */, const auto& feedback) {
    // Show feedback->hint and feedback->progress_* to the operator.
});

// Invoke explicitly, for example from a UI button, after sufficient data is
// collected. finish() throws until the first action feedback provides goal_id.
client.finish();
```
