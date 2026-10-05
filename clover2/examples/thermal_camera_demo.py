import numpy as np
from clover2 import Clover2

drone = Clover2()

print("Waiting for thermal camera...")

temperature = drone.thermal_camera.get_temperature()

min_y, min_x = np.unravel_index(np.argmin(temperature), temperature.shape)
max_y, max_x = np.unravel_index(np.argmax(temperature), temperature.shape)
center_y, center_x = temperature.shape[0] // 2, temperature.shape[1] // 2

print(
    f"Min: {temperature[min_y, min_x]:.2f} °C at ({min_x}, {min_y}); "
    f"max: {temperature[max_y, max_x]:.2f} °C at ({max_x}, {max_y}); "
    f"center: {temperature[center_y, center_x]:.2f} °C "
    f"at ({center_x}, {center_y})"
)
