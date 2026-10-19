# Camera calibration

Запустить calibration node:

```bash
ros2 run clover2_camera_utils camera_calibration \
  --ros-args \
  -p capture_timeout:=5.0 \
  -p image_timeout:=10.0
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

Нода автоматически оценивает каждый кадр с найденной доской и добавляет только
полезные недублирующиеся observations. Перемещайте доску по кадру, меняя её
положение, размер и наклон. Когда feedback показывает `data_sufficient: true`,
завершите калибровку:

```bash
ros2 service call \
  /camera_calibration/finish \
  clover2_camera_utils_msgs/srv/FinishCalibration \
  "{}"
```