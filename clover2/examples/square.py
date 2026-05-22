import math
import time

from clover2 import Clover2

drone = Clover2()

NAN = float("nan")
square_points = [(NAN, -1), (-1, NAN), (NAN, 1), (1, NAN)]

time.sleep(1)
drone.navigate_wait("base_link", z=1.5, speed=0.4)

for x, y in square_points:
    time.sleep(1)
    drone.navigate_wait("base_link", x=x, y=y, speed=0.4)

time.sleep(5.0)

drone.land()
