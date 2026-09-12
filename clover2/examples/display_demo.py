from clover2 import Clover2
from sensor_msgs.msg import Image


def make_test_image(width: int, height: int) -> Image:
    image = Image()
    image.width = width
    image.height = height
    image.encoding = 'mono8'
    image.is_bigendian = False
    image.step = width

    pixels = bytearray(width * height)
    for y in range(height):
        for x in range(width):
            if (
                x == 0
                or x == width - 1
                or y == 0
                or y == height - 1
                or x == y
                or x == width - y - 1
            ):
                pixels[y * width + x] = 255

    image.data = pixels
    return image


drone = Clover2()

if drone.display is None:
    raise RuntimeError('Display driver is not available')

display = drone.display
if 'mono8' not in display.supported_encodings:
    raise RuntimeError(
        f'Display does not support mono8: {display.supported_encodings}'
    )

print(
    f'Display: {display.width}x{display.height}, '
    f'max_fps={display.max_fps:.1f}, '
    f'encodings={display.supported_encodings}'
)
display.send_image(make_test_image(display.width, display.height))
print('Test image sent.')
