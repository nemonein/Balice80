#!/usr/bin/env python

import sys

import hid

vid = 0xFEED
pid = 0x1109
usage_page = 0xFF31
usage_id = 0x74


with hid.Device(vid, pid) as h:
    print(f"Device manufacturer: {h.manufacturer}")
    print(f"Product: {h.product}")
    print(f"Serial Number: {h.serial}")
    h.write(bytes([0x00, 0x01]))
    h.close()

sys.exit(0)
