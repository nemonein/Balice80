#!/usr/bin/env python

import hid


def pad_message(payload: bytes) -> bytes:
    if len(payload) > EP_SIZE:
        raise ("payload is too large: maximum payload is", str(EP_SIZE))
    return payload + b"\x00" * (EP_SIZE - len(payload))


def to_bytes(data: str) -> bytes:
    return data.encode()


EP_SIZE = 32
vid = 0xFEED
pid = 0x1109
usage_page = 0xFF60
usage_id = 0x0061
keyboard: hid.Device = None

for device in hid.enumerate():
    # print(
    #     f"Path: {device['path']}, Interface: {device['interface_number']}, Usage Page: {hex(device['usage_page'])}"
    # )
    if (
        device["vendor_id"] == vid
        and device["product_id"] == pid
        and device["usage_page"] == usage_page
        and device["usage"] == usage_id
    ):
        keyboard = hid.Device(path=device["path"])

if keyboard == None:
    print("Keyboard was not found.")
    exit(1)

print("Keyboard found!")
print("Product:", keyboard.product)
print("Manufacturer:", keyboard.manufacturer)

# message = pad_message(b"Change the layer")

# 이렇게 해야 16진수로 01 02 00 00 00 ........ 00 이 나가게 된다.
# 또, 이건 다음 명령과 같은 결과를 낸다.
# hidapitester --vidpid feed:1109 --usagePage 0xFF60 --open --length 32 --send-output 1,2 --close
message = pad_message(bytes([0x01, 0x02]))

print("sending:", message)
keyboard.write(message)
# keyboard.write(b"\x00" + message)
# print("reading...")
# received = keyboard.read(EP_SIZE)
# print("received:", received)

keyboard.close()
print("device closed")
