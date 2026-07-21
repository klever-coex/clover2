import time

from clover2 import Clover2

drone = Clover2()

print("Rainbow (5s)")
drone.rainbow(period=2.0, duration=5.0)
time.sleep(5.5)

print("Blink (5s, period=0.5)")
drone.blink(255, 255, 255, period=0.5, duration=5.0)
time.sleep(5.5)

for name, r, g, b in [
    ("red", 255, 0, 0),
    ("green", 0, 255, 0),
    ("blue", 0, 0, 255),
    ("yellow", 255, 255, 0),
    ("magenta", 255, 0, 255),
    ("cyan", 0, 255, 255),
]:
    print(f"Solid {name} (1s)...")
    drone.solid_color(r, g, b, duration=1.0)
    time.sleep(1.2)

print("Clearing strip.")
drone.clear()
