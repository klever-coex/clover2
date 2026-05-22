log_info "Copy camera calibration files"
mkdir -p /home/$USER/.ros/camera_info
cp $ASSETS_DIR/camera_info/*.yaml /home/$USER/.ros/camera_info/
