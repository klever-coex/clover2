import clover2
import time

drone = clover2.Clover2()

drone.turn_motors_on()

print("takeoff")
drone.move(z=2.0, frame_id="base_link")

time.sleep(10)

print("state1")
drone.move(x=0.5, frame_id="base_link")

time.sleep(10)

print("state2")
drone.move(x=0.5, frame_id="base_link")

time.sleep(10)

print("state3")
drone.move(x=0.5, frame_id="base_link")

time.sleep(10)

print("state4")
drone.move(z=1.0)

time.sleep(10)

drone.land()

print(drone.get_battery())

time.sleep(2)
