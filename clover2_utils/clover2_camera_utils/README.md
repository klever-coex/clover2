# Camera calibration

Запустить calibration node:

```bash
ros2 run clover2_camera_utils camera_calibration
```

Запуск с feedback:

```bash
ros2 action send_goal \
  --feedback \
  /camera_calibration/calibrate \
  clover2_camera_utils_msgs/action/CalibrateCamera \
  "{
    image_topic: '/webcam/camera/image_raw',
    calibration_method: 'pinhole',
    board_type: 'chessboard',
    board_rows: 6,
    board_columns: 9,
    square_size_m: 0.0225
  }"
```

При корректном распознавании, выполнять:

```bash
ros2 service call \
  /camera_calibration/capture_sample \
  clover2_camera_utils_msgs/srv/CaptureCalibrationSample \
  "{}"
```

Когда feedback показывает `data_sufficient: true` для завершения калибровки:

```bash
ros2 service call \
  /camera_calibration/finish \
  clover2_camera_utils_msgs/srv/FinishCalibration \
  "{}"
```