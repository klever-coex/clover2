import math
import time

from clover2 import Clover2

drone = Clover2()

nan = float("nan")
square_points = [(nan, -2), (-2, nan), (nan, 2), (2, nan)]

time.sleep(1)
drone.offboard.navigate_async("base_link", z=2, speed=1.0)

time.sleep(1)
drone.offboard.navigate_async("map", x=0, y=0, z=1, speed=0.5)

for x, y in square_points:
    time.sleep(1)
    drone.offboard.navigate_async("base_link", x=x, y=y, speed=0.8)

time.sleep(5.0)

drone.land()
