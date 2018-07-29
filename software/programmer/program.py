#!/usr/bin/env python
''' DCDarkNet badge programmer script!

    This script connects to a badge micro through an ST-Link device.
    It reads the UID and checks if the keys have already been programmed.

    If they keys have not been programmed, the UID is used to check the key
    DB file to see if the device has previously been programmed. If the device
    has been previously programmed, the same keys are flashed back on it.

    If the device had not been previously programmed, a new key file is
    selected and flashed onto the device. Once the keys have been verified
    written, the key DB file is updated and the key file is moved to the 
    used/ directory.

    TODO: If a main flash file is provided, it is also programmed into the device.
'''

import os
import csv
import re
import shutil
import time
import math
import argparse
import sys
from openocd.flashProgrammer import flashProgrammer


MAIN_FLASH_ADDR = 0x8000000
FLASH_BASE = 0x8000000
KEY_FLASH_OFFSET = 0xC000
#SECTOR_SIZE = 0x800

def get_used_key_dir(key_dir):
    return key_dir + '/used'

def get_key_db_file(key_dir):
    return key_dir + '/used_keys.csv'

def initialSetup(key_dir, used_key_dir, key_db_file):
    """ Make sure all required files and directories are present """

    if not os.path.exists(key_dir):
        raise IOError('Key directory not found: ' + key_dir)

    if not os.path.exists(used_key_dir):
        print('Used key dir not found, creating.')
        os.makedirs(used_key_dir)

    if not os.path.exists(key_db_file):
        print('Used key database file not found, creating.')
        with open(key_db_file, 'a') as dbfile:
            dbfile.write('uid,keyfile,timestamp\n')
            dbfile.close()


def readDB(filename):
    dbdict = {}
    with open(filename, 'r') as dbfile:
        reader = csv.reader(dbfile)

        next(reader, None)  # skip the header
        for row in reader:
            uid = row[0]
            dbdict[uid] = {'filename': row[1], 'timestamp': int(row[2])}

    return dbdict


def updateDB(key_dir, filename, uid, key_file):
    
    used_key_dir = get_used_key_dir(key_dir)

    # Move file to used dir
    key_filename = key_dir + '/' + key_file
    shutil.move(key_filename, used_key_dir + '/')

    # Update db file
    with open(filename, 'a') as dbfile:
        dbfile.write('{},{},{}\n'.format(uid, key_file, int(time.time())))
        dbfile.close()


def readKeyFiles(keydir):
    keydir_list = os.listdir(keydir)
    keyfiles = []

    for filename in keydir_list:
        # Only add filenames that match the 4 hex digits regex
        if re.match(r'^[0-9A-Fa-f]{4}$', filename):
            keyfiles.append(filename)

    return keyfiles


def readUID(flasher):
    # Read device unique ID
    #stm32f411 0x1FFF7A10
    #define ID_FLASH_ADDRESS       0x1FFF7A22
    #define ID_DBGMCU_IDCODE      0xE0042000
    #uid_bytes = flasher.readMem(0x1FFFF7E8, 12)
    uid_bytes = flasher.readMem(0x1FFF7A10, 12)
    uid = ''
    for byte in range(len(uid_bytes)):
        uid += '{:02X}'.format(uid_bytes[byte])
    
    return uid


def dcdcCheck(flasher):
    dcdc_bytes = flasher.readMem(FLASH_BASE + KEY_FLASH_OFFSET, 2)

    if dcdc_bytes[0] == 0xdc and dcdc_bytes[1] == 0xdc:
        return True
    else:
        return False


#def roundToSectorSize(size):
#    return int(math.ceil(float(size)/float(SECTOR_SIZE)) * SECTOR_SIZE)


def programKeyfile(flasher, key_filename):

    key_flash_size = os.path.getsize(key_filename)

    # flasher.erase(FLASH_BASE + KEY_FLASH_OFFSET, roundToSectorSize(key_flash_size))

    flasher.flashFile(key_filename, FLASH_BASE + KEY_FLASH_OFFSET)

    if flasher.verifyFile(key_filename, KEY_FLASH_OFFSET):
        return True
    else:
        return False


def programMainFlash(flash_filename):
    if flash_filename is not None:
        print('Programming main flash')

        flasher._sendCmd('reset halt')  # Make sure the processor is stopped

        # Erase everything until the key
        # flasher.erase(FLASH_BASE, KEY_FLASH_OFFSET)
        flasher.eraseSector(0,0,0)
        flasher.eraseSector(0,4,7)

        if '.bin' in flash_filename:
            # If a .bin file is used, we need to specify the base address
            flasher.flashFile(flash_filename, FLASH_BASE)
        else:
            # .elf and .hex files have address informatino in them so use 0
            flasher.flashFile(flash_filename, 0)

        if flasher.verifyFile(flash_filename, 0):
            print('Flash programmed succesfully')
            return True
        else:
            print('Error prorgamming flash')
            return False

parser = argparse.ArgumentParser()
parser.add_argument('--openocd_dir', action='store', default='/opt/gnuarmeclipse/openocd/0.10.0-201601101000-dev/', help='Open OCD dev directory')
parser.add_argument('--key_dir', action='store', required=True, help='key directory')
parser.add_argument('--flash', action='store', help='hex file to program')


args, unknown = parser.parse_known_args()

key_dir = args.key_dir
used_key_dir = get_used_key_dir(key_dir)
key_db_file = get_key_db_file(key_dir)

initialSetup(key_dir, used_key_dir, key_db_file)
dbdict = readDB(key_db_file)
unused_keys = readKeyFiles(key_dir)

try:

    flasher = flashProgrammer(args.openocd_dir)

    if flasher.connected is True:
        print('Connected to openOCD')

        flasher._sendCmd('reset halt')  # Make sure the processor is stopped  

        uid = readUID(flasher)
        print('Device UID is ' + uid)
        
        # Check if key is already present in this device
        if dcdcCheck(flasher) is False:
            
            # Check database to see if we've already flashed this device
            if uid in dbdict:
                # Already used
                key_file = dbdict[uid]['filename']
                key_filename = used_key_dir + '/' + key_file
                if programKeyfile(flasher, key_filename) is True:
                    print('Key programmed successfully')
                else:
                    print('Error programming key')
                
            else:
                key_file = unused_keys.pop()
                key_filename = key_dir + '/' + key_file
                if programKeyfile(flasher, key_filename) is True:
                    updateDB(key_dir, key_db_file, uid, key_file) 
                    print('Key programmed successfully')
                else:
                    print('Error programming key')

        else:
            print('Key already programmed')

        # programMainFlash()
        if args.flash:
            programMainFlash(args.flash)

    else:        
        raise IOError('Could not connect to flash programmer')

except:
    raise

finally:
    # Make sure we kill the flasher process, otherwise openocd thread 
    # stays open in background
    if flasher:
        flasher.kill()
