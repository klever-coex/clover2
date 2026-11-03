# Camera calibration

Start the calibration node:

```bash
ros2 run clover2_camera_utils camera_calibration \
  --ros-args \
  -p processing_rate:=5.0
```

`processing_rate` specifies how often the board detector processes image frames.

The action goal selects the board type. Its geometry is stored in a JSON
profile on the host running `camera_calibration`. Example profiles are installed
in
`share/clover2_camera_utils/config/boards`:

```json
{
  "rows": 6,
  "columns": 9,
  "square_size_m": 0.0225
}
```

For ChArUco, use a profile with the number of squares, marker size, and ArUco
dictionary:

```json
{
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
    camera_namespace: '/webcam',
    calibration_method: 'pinhole',
    board_type: 'chessboard',
    board_config_path: '/absolute/path/to/chessboard_6x9_22_5mm.json'
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
