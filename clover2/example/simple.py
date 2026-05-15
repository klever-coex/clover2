import math

from clover2 import Clover2

drone = Clover2()

drone.offboard.navigate_async("base_link", z=1.5, speed=0.5)

drone.offboard.navigate_async("map", yaw=0, speed=0.5)

drone.offboard.navigate_async("base_link", y=1.0, speed=1.5)

# import time

# time.sleep(10.0)

# drone.offboard.navigate_async(y=4, speed=0.5)
# drone.offboard.navigate_async(x=4, speed=0.5)
# drone.offboard.navigate_async(y=0, speed=0.5)
# drone.offboard.navigate_async(x=0, speed=0.5)

drone.land()
