#!/usr/bin/env python

import telnetlib
import subprocess
import threading
import time

class openOCDThread(threading.Thread):
    ''' Run openocd as subprocess and read output in separate thread
    '''
    def __init__(self, openocd_dir, verbose=True):
        super(openOCDThread, self).__init__()
        self.proc = None
        self.ready = False
        self.verbose = verbose
        self.openocd_dir = openocd_dir

    def run(self):
        cmd = [self.openocd_dir + 'bin/openocd']
        cmd += ['-f', self.openocd_dir + 'scripts/interface/stlink-v2.cfg']
        cmd += ['-f', self.openocd_dir + 'scripts/target/stm32f4x.cfg']

        self.proc = subprocess.Popen(cmd, stderr=subprocess.PIPE)

        while self.proc.poll() is None:

            l = self.proc.stderr.readline() # This blocks until it receives a newline.
            if 'stm32f4x.cpu: hardware has 6 breakpoints, 4 watchpoints' in l:
                self.ready = True

            if self.verbose:
                print(l.strip())
 
        if self.verbose:
            print self.proc.stderr.read()

    def kill(self):
        self.proc.terminate()


class flashProgrammer(object):
    def __init__(self, openocd_dir):

        self.connected = False

        # Start openOCD as daemon so it will automatically close on program exit
        self.openOCD = openOCDThread(openocd_dir)
        self.openOCD.daemon = True
        self.openOCD.start()

        timeout = 500

        # Don't connect telnet until openOCD is ready
        while (timeout > 0) and (self.openOCD.ready is False):
            time.sleep(0.01)
            timeout -= 1

        if self.openOCD.ready is True:
            self.telnet = telnetlib.Telnet('127.0.0.1', 4444)
            self.telnet.read_until('\r>', timeout=1)  # Wait until prompt read
            self.connected = True
        else:
            print('Unable to connect to openOCD server')

    def _sendCmd(self, cmd, timeout=10):
        ''' Send command over telnet and capture output
            return list of output lines without command echo
        '''

        self.telnet.write(cmd + '\n')

        lines = self.telnet.read_until('\n\r>', timeout=timeout)

         # Remove prompt and split into lines
        lines = lines.strip('\n\r>').split('\r\n')

        # Remove command echo
        lines.pop(0)

        return lines

    def readMem(self, address, size):
        ''' Returns list of bytes at address
        '''
        lines = self._sendCmd('mdb ' + str(address) + ' ' + str(size))

        data = []

        for line in lines:
            addr_str, data_str = line.split(': ')
            addr = int(addr_str, 16)

            for byte in data_str.strip().split(' '):
                data.append(int(byte, 16))

        return data

    def erase(self, address, size):
        lines = self._sendCmd('flash erase_address ' + str(address) + ' ' + str(size))
        # TODO - verify output
        print(lines)
    
    def eraseSector(self, bank, sector_begin, sector_end):
        lines = self._sendCmd('flash erase_sector ' + str(bank) + ' ' + 
                str(sector_begin) + ' ' + str(sector_end))
        print(lines)

    def flashFile(self, filename, address):
        #lines = self._sendCmd('flash write_image erase ' +  
        lines = self._sendCmd('flash write_image ' +  
                    ' ' + filename + ' ' + str(address), timeout=60)
        # TODO - verify output
        print(lines)

    def dumpImage(self, filename, address, size):
        lines = self._sendCmd('dump_image ' + filename + ' ' + str(address) + ' ' + str(size))
        # TODO - verify output
        print(lines)

    def verifyFile(self, filename, address):
        lines = self._sendCmd('verify_image ' + filename + ' ' + str(address))
        
        ''' Valid output example:
            stm32f1x.cpu: target state: halted
            target halted due to breakpoint, current mode: Handler HardFault
            xPSR: 0x61000003 pc: 0x2000002e msp: 0x20004fd8
            verified 44 bytes in 0.019517s (2.202 KiB/s)

            Invalid output example:
            stm32f1x.cpu: target state: halted
            target halted due to breakpoint, current mode: Handler HardFault
            xPSR: 0x61000003 pc: 0x2000002e msp: 0x20004fd8
            checksum mismatch - attempting binary compare
            diff 0 address 0x0000d400. Was 0x00 instead of 0xdc
            diff 1 address 0x0000d401. Was 0x00 instead of 0xdc
            No more differences found.
        '''

        # Last line has the 'verified' result 
        if 'verified' in lines[len(lines)-1]:
            return True
        else:
            return False


    def kill(self):
        self.openOCD.kill()

