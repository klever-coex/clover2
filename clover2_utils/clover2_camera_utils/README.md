# Camera calibration

Запустить calibration node:

```bash
ros2 run clover2_camera_utils camera_calibration \
  --ros-args \
  -p processing_rate:=5.0
```

Запуск с feedback:

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

После успешного завершения нода передаёт рассчитанные параметры в
`/camera_namespace/set_camera_info` (`sensor_msgs/srv/SetCameraInfo`). Если
сервис драйвера недоступен или отклоняет запрос, калибровка всё равно завершается
успешно: рассчитанные параметры возвращаются в result action и печатаются в лог.

Нода автоматически анализирует входной поток с частотой `processing_rate`
(по умолчанию 5 Гц) и добавляет только полезные недублирующиеся кадры.

Завершение
```bash
ros2 service call \
  /camera_calibration/finish \
  clover2_camera_utils_msgs/srv/FinishCalibration \
  "{}"
```