import cv2
from clover2 import Clover2

drone = Clover2()
detector = cv2.QRCodeDetector()

print("Waiting for first QR code...")

while True:
    img = drone.get_image()
    data, bbox, _ = detector.detectAndDecode(img)

    if data:
        print(f"QR Code detected: {data}")
        break
