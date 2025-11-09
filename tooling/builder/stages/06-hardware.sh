log_info "Edit /boot/firmware/config.txt"

# Enable camera detection
echo 'camera_auto_detect=1' | sudo tee -a /boot/firmware/config.txt

# Fan config
echo 'dtparam=fan_temp0=35000' | tee -a /boot/firmware/config.txt
echo 'dtparam=fan_temp0_hyst=5000' | tee -a /boot/firmware/config.txt
echo 'dtparam=fan_temp0_speed=175' | tee -a /boot/firmware/config.txt

# Enable UART0
echo 'dtoverlay=uart0-pi5' | tee -a /boot/firmware/config.txt
