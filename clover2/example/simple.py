import clover2
import time

drone = clover2.Clover2()

drone.turn_motors_on()
time.sleep(2)

print("takeoff")
drone.move(x=.0, y=.0, z=0.7, speed=0.3,frame_id="map")

time.sleep(10)

drone.move(z=1.3, frame_id="map")

time.sleep(10)

drone.move(z=1.3, y=-0.5, frame_id="map")

time.sleep(10)

drone.move(x=1.0, z=0.7, speed=0.1, frame_id="map")

time.sleep(10)

drone.land()

print(drone.get_battery())

time.sleep(10)
