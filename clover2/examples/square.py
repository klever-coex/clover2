import math
import time

from clover2 import Clover2

drone = Clover2()

NAN = float("nan")
square_points = [(NAN, 2), (2, NAN), (NAN, -2), (-2, NAN)]

time.sleep(1)
drone.navigate_wait("base_link", z=1, speed=1.0)

for x, y in square_points:
    time.sleep(1)
    drone.navigate_wait("base_link", x=x, y=y, speed=0.8)

time.sleep(5.0)

drone.land()
