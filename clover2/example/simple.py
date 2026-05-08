import clover2

drone = clover2.Clover2()

drone.offboard.navigate_async("map", x=0.0, y=0.0, speed=2.0)

# import time

# time.sleep(10.0)

# drone.offboard.navigate_async(y=4, speed=0.5)
# drone.offboard.navigate_async(x=4, speed=0.5)
# drone.offboard.navigate_async(y=0, speed=0.5)
# drone.offboard.navigate_async(x=0, speed=0.5)

# drone.land()
