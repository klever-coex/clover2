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

`calibration_client` wraps the calibration action and finish service. Each
`start()` returns a shared handle for its own session. The goal UUID is assigned
when the action server accepts the goal, so call `finish()` after acceptance.

```cpp
#include <clover2_camera_utils/calibration_client.hpp>
#include <string>

clover2_camera_utils::calibration_client client(node);

auto make_goal = [](const std::string& camera_namespace) {
    clover2_camera_utils::calibration_client::calibrate_camera::Goal goal;
    goal.camera_namespace = camera_namespace;
    goal.calibration_method = "fisheye";
    goal.target_config_path = "/absolute/path/to/target.json";
    return goal;
};

auto front = client.start(make_goal("/front_camera/camera"),
    [](auto /* goal_handle */) {
        // Goal was accepted; the session now has its ROS goal ID.
    },
    [](auto /* goal_handle */, const auto& feedback) {
        // Show feedback->hint and feedback->progress_* to the operator.
});
auto rear = client.start(make_goal("/rear_camera/camera"));

// Each handle finishes only its corresponding camera session. The goal must
// have been accepted before finish() is called.
front->finish();
rear->finish();

// Or retrieve active sessions and finish one by index:
auto sessions = client.active_sessions();
sessions.front()->finish();
```

```cpp
auto session = client.start(
    make_goal("/front_camera/camera"),
    nullptr,  // Goal response callback.
    nullptr,  // Feedback callback.
    [](const auto& result) {
        // Inspect result.code and result.result->success/message.
        // Display result.result->camera_info and calibration quality on success.
    });

// Later, after goal acceptance:
auto pending = session->finish();
```
