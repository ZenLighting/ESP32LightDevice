import struct
import sys

light_length = int(sys.argv[1])
pin = int(sys.argv[2])
brightness = int(sys.argv[3])

rgbstr = sys.argv[4]
(r, g, b,) = rgbstr.split(",")
(r, g, b) = (int(r), int(g), int(b))

print(r, g, b)
with open("./data/lightconfig.txt", "wb") as fi:
    bytes_to_write = struct.pack("!BBBBBB", light_length, pin, brightness, r, g, b)
    fi.write(bytes_to_write)