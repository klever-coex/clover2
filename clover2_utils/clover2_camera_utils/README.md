# Camera calibration

Start the calibration node:

```bash
ros2 run clover2_camera_utils camera_calibration \
  --ros-args \
  -p processing_rate:=5.0
```

`processing_rate` specifies how often the board detector processes image frames.

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
    board_rows: 6,
    board_columns: 9,
    square_size_m: 0.0225
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
