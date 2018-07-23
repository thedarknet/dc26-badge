#!/usr/bin/env python
# -*- coding: utf-8 -*-

from openocd.flashProgrammer import flashProgrammer

# 
# Quick example of reading UID, erasing flash, programming flash, and reading back flash 
# 
flasher = flashProgrammer()

if flasher.connected is True:
    flasher._sendCmd('reset halt')

    # Read device unique ID
    uid_bytes = flasher.readMem(0x1FFFF7E8, 12)
    uid = ''
    for byte in range(len(uid_bytes)):
    	uid += '{:02X}'.format(uid_bytes[byte])
    print('uid:')
    print(uid)

    flasher.erase(0x800d400, 0x400)

    flasher.flashFile('/Users/alvaro/Desktop/test.bin', 0x800d400)

    print flasher.verifyFile('/Users/alvaro/Desktop/test.bin', 0x800d400)
    print flasher.verifyFile('/Users/alvaro/Desktop/flash.bin', 0x800d400)

    # flasher.dumpImage('/Users/alvaro/Desktop/flash.bin', 0x800d400, 0x400)

flasher.kill()

