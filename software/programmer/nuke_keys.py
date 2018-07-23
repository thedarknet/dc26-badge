#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# This will erase the keys! (it will print them out before doing it though...)
#

import argparse
from openocd.flashProgrammer import flashProgrammer

MAIN_FLASH_ADDR = 0x8000000
FLASH_BASE = 0x8000000
KEY_FLASH_OFFSET = 0x1ffd4
SECTOR_SIZE = 0x800
KEY_BYTES = 30

parser = argparse.ArgumentParser()
parser.add_argument('--openocd_dir', action='store', default='/home/cmdc0de/opt/gnuarmeclipse/openocd/0.10.0-201701241841/', help='Open OCD dev directory')

args, unknown = parser.parse_known_args()

try:
    flasher = flashProgrammer(args.openocd_dir)

    if flasher.connected is True:
        flasher._sendCmd('reset halt')

        # Read device unique ID
        uid_bytes = flasher.readMem(0x1FFFF7E8, 12)
        uid = ''
        for byte in range(len(uid_bytes)):
        	uid += '{:02X}'.format(uid_bytes[byte])
        print('uid:')
        print(uid)

        # Read device unique ID
        key_bytes = flasher.readMem(FLASH_BASE + KEY_FLASH_OFFSET, KEY_BYTES)
        key = ''
        for byte in range(len(key_bytes)):
            key += '{:02X}'.format(key_bytes[byte])
        print('key:')
        print(key)

        print('Nuking keys!')
        flasher.erase(FLASH_BASE + 0x20000 - SECTOR_SIZE, SECTOR_SIZE)

finally:
    # Make sure we kill the flasher process, otherwise openocd thread 
    # stays open in background
    if flasher:
        flasher.kill()   
