import clover2

drone = clover2.Clover2()

drone.offboard.navigate_async("map", z=2.0, speed=.5)

# import time

# time.sleep(10.0)

# drone.offboard.navigate_async(y=4, speed=0.5)
# drone.offboard.navigate_async(x=4, speed=0.5)
# drone.offboard.navigate_async(y=0, speed=0.5)
# drone.offboard.navigate_async(x=0, speed=0.5)

# drone.land()
