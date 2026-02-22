log_info "Edit /boot/firmware/config.txt"

# ================= RPI 5 section =====================
echo '[pi5]' | sudo tee -a /boot/firmware/config.txt

# Enable camera detection
echo 'camera_auto_detect=1' | sudo tee -a /boot/firmware/config.txt

# Fan config
echo 'dtparam=fan_temp0=40000,fan_temp0_hyst=5000,fan_temp0_speed=125' | sudo tee -a /boot/firmware/config.txt
echo 'dtparam=fan_temp1=55000,fan_temp1_hyst=4000,fan_temp1_speed=200' | sudo tee -a /boot/firmware/config.txt
echo 'dtparam=fan_temp2=80000,fan_temp2_hyst=3000,fan_temp2_speed=255' | sudo tee -a /boot/firmware/config.txt

# Enable UART0
echo 'dtoverlay=uart0-pi5' | sudo tee -a /boot/firmware/config.txt

# ================= RPI CM5 section =====================

# Enable camera detection
echo 'camera_auto_detect=0' | sudo tee -a /boot/firmware/config.txt

# Fan config
echo 'dtparam=fan_temp0=40000,fan_temp0_hyst=5000,fan_temp0_speed=125' | sudo tee -a /boot/firmware/config.txt
echo 'dtparam=fan_temp1=55000,fan_temp1_hyst=4000,fan_temp1_speed=200' | sudo tee -a /boot/firmware/config.txt
echo 'dtparam=fan_temp2=80000,fan_temp2_hyst=3000,fan_temp2_speed=255' | sudo tee -a /boot/firmware/config.txt

# Enable UART0
echo 'dtoverlay=uart0-pi5' | sudo tee -a /boot/firmware/config.txt

# Enable camera
echo 'dtoverlay=imx219,cam0' | sudo tee -a /boot/firmware/config.txt
echo 'dtoverlay=ov5647,cam0' | sudo tee -a /boot/firmware/config.txt

# Disable console on UART
sudo sed -i -e 's/\(^\| \)console=serial0,115200\( \|$\)/ /g' -e 's/  */ /g' -e 's/^ //;s/ $//' /boot/firmware/cmdline.txt
