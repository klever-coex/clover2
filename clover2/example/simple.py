import clover2
import time

drone = clover2.Clover2()

time.sleep(5)

drone.turn_motors_on()

drone.move(
    z=1.0,
)

time.sleep(5)

drone.land()

print(drone.get_battery())

time.sleep(2)
