/**
 * Darknet "Official Eggplant Emoji Shitty Add-on (SAO) board
 *
 * Designed by Krux - 2018
 *
 */

/**
 * Pin notes by Suovula, see also http://hlt.media.mit.edu/?p=1229
 *
 * DIP and SOIC have same pinout, however the SOIC chips are much cheaper, especially if you buy more than 5 at a time
 * For nice breakout boards see https://github.com/rambo/attiny_boards
 *
 * Basically the arduino pin numbers map directly to the PORTB bit numbers.
 *
// I2C
arduino pin 0 = not(OC1A) = PORTB <- _BV(0) = SOIC pin 5 (I2C SDA, PWM)
arduino pin 2 =           = PORTB <- _BV(2) = SOIC pin 7 (I2C SCL, Analog 1)
// Timer1 -> PWM
arduino pin 1 =     OC1A  = PORTB <- _BV(1) = SOIC pin 6 (PWM)
arduino pin 3 = not(OC1B) = PORTB <- _BV(3) = SOIC pin 2 (Analog 3)
arduino pin 4 =     OC1B  = PORTB <- _BV(4) = SOIC pin 3 (Analog 2)
 */
#define I2C_SLAVE_ADDRESS 0x36 // the 7-bit address (36 = EG for Eggplant *snerk*)
// Get this from https://github.com/rambo/TinyWire
#include <TinyWireS.h>
// The default buffer size, though we cannot actually affect it by defining it in the sketch
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif

#include <EEPROM.h>
#include "EEPROMAnything.h"

#define LED_PB3 3
#define LED_PB4 4

// version info
char TYPE[9] = "EGGPLANT";
char VERSION[9] = "1.0.0";
byte OPTIONS[9] = "";
char SERIALNUM[9] = "";
byte write_serial = 0;
char NAME[13] = "NonameEggie";

// led vars
byte led_mode = 0;
byte led_save_mode = 0;
byte led_mode_set = 0;

int blue = 0;
int red = 0;
int blue_mode = 1;
int red_mode = 1;
int blue_counter = 0;
int red_counter = 0;
int blue_counter_max = 1;
int red_counter_max = 4;
int blue_flip_counter = 0;
int red_flip_counter = 0;
int blue_min = 12;
int red_min = 0;
int blue_max = 140;
int red_max = 90;
int led_cycle_max = 96;

// random seed
byte seed_set = 0;

// eeprom operations
unsigned int ee_address = 0;
byte ee_bytes = 0;
byte ee_data = 0;
byte ee_index = 0;
char TEMP_STR[8] = {};

// The "registers" we expose to I2C
volatile byte i2c_regs[TWI_RX_BUFFER_SIZE] = {};
// Tracks the current register pointer position
volatile byte reg_position;
// Tracks wheter to start a conversion cycle
volatile boolean start_conversion;
// Counter to track where we are averaging
byte avg_count;
// Some temp value holders
int avg_temp1;
int avg_temp2;



void sendString(String toSend)
{ 
  int c = toSend.length();
  for (int x = 0; x < c; x++)
  {
    TinyWireS.send(toSend[x]);
  }
}

/**
 * This is called for each read request we receive, never put more than one byte of data (with TinyWireS.send) to the 
 * send-buffer when using this callback
 */
void requestEvent()
{ 
  // random seed based on I2C event time
  if(!seed_set)
  {
    randomSeed(micros());
    seed_set = 1;
  }
  
  // max send 16 bytes - else crash
   
  switch(i2c_regs[0])
  {
    case 0:
      // Read System Info
      switch(i2c_regs[1])
      {
        case 0:
          // Type
          sendString(TYPE);
                      
          break;
        case 1:
          // Version
          sendString(VERSION);
        
          break;
        case 2:
          // Serial

          if(SERIALNUM[0] < 32 || SERIALNUM[0] > 126)
          {
            // invalid serial number, so generate a new one
            for (int i = 0 ; i < 8 ; i++)
            {
              SERIALNUM[i] = random(48,63);
              if(SERIALNUM[i] > 57)
              {
                SERIALNUM[i] = SERIALNUM[i] + 7;
              }
            }
            SERIALNUM[9] = '\0';
          }
          
          sendString(SERIALNUM);
        
          break;
        case 3:
          // Name
          sendString(NAME);

          break;
        default:
          sendString("ERROR 0");
          break;
      }

      break;
    case 1:
      // Write System Info
      switch(i2c_regs[1])
      {
        case 3:
          // Name
          memset(NAME, 0, sizeof NAME);
          ee_index = 2;
          while(ee_index < 14)
          {
            if(i2c_regs[ee_index] >= 32 && i2c_regs[ee_index] <=126)
            {
              NAME[ee_index - 2] = i2c_regs[ee_index];
            }
            else
            {
              NAME[ee_index - 2] = '\0';
              // no more chars
              ee_index = 14;
            }
            ee_index++;
          }

          EEPROM_writeAnything(36,NAME);
          sendString(NAME);
          sendString("|OK");
      
          break;
        default:
          sendString("ERROR 1");
          break;
      }
          
      break;
    case 2:
      // Read LED Mode
      TinyWireS.send(led_mode);
      TinyWireS.send(red);
      TinyWireS.send(blue);
      TinyWireS.send(red_flip_counter);
      TinyWireS.send(blue_flip_counter);
      TinyWireS.send(red_counter);
      TinyWireS.send(blue_counter);
      TinyWireS.send(red_counter_max);
      TinyWireS.send(blue_counter_max);
      TinyWireS.send(red_min);
      TinyWireS.send(blue_min);
      TinyWireS.send(red_max);
      TinyWireS.send(blue_max);
      TinyWireS.send(led_cycle_max);
      
      break;
    case 3:
      // Write LED Mode
      if(i2c_regs[1] > 5)
      {
        i2c_regs[1] = 0;
      }
      led_mode = i2c_regs[1];
      led_save_mode = i2c_regs[1];
      led_mode_set = 0;
      OPTIONS[0] = led_mode;
      EEPROM.write(18,OPTIONS[0]);
      
      TinyWireS.send(led_mode);
      sendString("|OK");
      
      break;
    case 4:
      // Read Infection Mode
      switch(i2c_regs[1])
      {
        case 0:
          // Try to infect me

          // grab our status and clean bits if needed based on percentage
          ee_data = OPTIONS[1];
          
          // Chlamydia bit 7, 50% transmission rate
          if(random(100) < 50)
          {
            // clear bit 7
            ee_data = ee_data & (~64);
          }

          // Herpes bit 8, 30% transmission rate
          if(random(100) < 70)
          {
            // clear bit 8
            ee_data = ee_data & (~128);
          }

          // send the result
          TinyWireS.send(ee_data);
          if(ee_data)
          {
            sendString("|INFECTED");
            led_mode = 16;
            led_mode_set = 0;
          }
          else
          {
            sendString("|AOK");
          }
          
          break;
        case 1:
          // Report true infection status
          TinyWireS.send(OPTIONS[1]);
          TinyWireS.send(OPTIONS[2]);
          if(OPTIONS[1] || OPTIONS[2])
          {
            sendString("|INFECTED");
            led_mode = 16;
            led_mode_set = 0;
          }
          else
          {
            sendString("|AOK");
          }

          break;
        default:
          sendString("ERROR 4");
      }
      
      break;
    case 5:
      // Write Infection Mode
      switch(i2c_regs[1])
      {
        case 0:
          // Infect

          // only act on bits 7 & 8
          i2c_regs[2] = i2c_regs[2] & 192;
          
          ee_data = OPTIONS[1];
          ee_data = ee_data | i2c_regs[2];
          if(ee_data != OPTIONS[1])
          {
            OPTIONS[1] = ee_data;
            EEPROM.write(19,OPTIONS[1]);
            led_mode = 16;
            led_mode_set = 0;
            TinyWireS.send(OPTIONS[1]);
            sendString("|INFECTED");

            // see if we have the Herpes
            ee_data = ee_data & 128;
            if(ee_data)
            {
              // we do, so save that bit in options to, as it can't be really ever cured
              OPTIONS[2] = ee_data;
              EEPROM.write(20,OPTIONS[2]);
            }
          }
          else
          {
            TinyWireS.send(OPTIONS[1]);
            sendString("|NOCHANGE");
          }
          
          break;
        case 1:
          // Cure

          // only act on bits 7 & 8
          i2c_regs[2] = i2c_regs[2] & 192;
          
          ee_data = OPTIONS[1];
          ee_data = ee_data & (~i2c_regs[2]);
          if(ee_data != OPTIONS[1])
          {
            OPTIONS[1] = ee_data;
            EEPROM.write(19,OPTIONS[1]);
            led_mode = 17;
            led_mode_set = 0;
            TinyWireS.send(OPTIONS[1]);
            sendString("|CURED");
          }
          else
          {
            TinyWireS.send(OPTIONS[1]);
            sendString("|NOCHANGE");
          }
          
          break;
        default:
          sendString("ERROR 5");
      }
      break;
    case 8:
      // Read EEPROM - address is Little-endian
      ee_address = (i2c_regs[2] << 8) | (i2c_regs[1]);
      ee_bytes = i2c_regs[3];
      if(ee_bytes > 16)
      {
        ee_bytes = 16;
      }
      while(ee_bytes > 0)
      {
        if(ee_address >= EEPROM.length()) {
          ee_address = 0;
        }
        ee_data = EEPROM.read(ee_address);
        TinyWireS.send(ee_data);
        ee_bytes--;
        ee_address++;
      }
      
      break;
    case 9:
      // Write EEPROM - address is Little-endian, limited to 10 bytes due to max message size, and odd i2c bugs
      ee_address = (i2c_regs[2] << 8) | (i2c_regs[1]);
      ee_bytes = i2c_regs[3];
      ee_index = 0;
      if(ee_bytes > 10)
      {
        ee_bytes = 10;
      }
      while(ee_bytes > 0)
      {
        if(ee_address >= EEPROM.length()) {
          ee_address = 0;
        }
        ee_data = i2c_regs[ee_index + 4];
        // this probably isn't the best way to do this, writing a byte at a time due to 100,000 write/erase cycles
        // but ideally we're not using this function a lot, and defcon is nigh
        EEPROM.write(ee_address, ee_data);
        TinyWireS.send(ee_data);
        ee_bytes--;
        ee_address++;
        ee_index++;
      }
      sendString("|OK");
      
      break;
    default:
      sendString("ERROR F");
  }
}

/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data, restart/stop) on the bus does
 * so be quick, set flags for long running tasks to be called from the mainloop instead of running them directly,
 */
void receiveEvent(uint8_t howMany)
{
  if (howMany < 1)
  {
    // Sanity-check
    return;
  }
  if (howMany > TWI_RX_BUFFER_SIZE)
  {
    // Also insane number
    return;
  }

  reg_position = TinyWireS.receive();
  howMany--;
  if (!howMany)
  {
    // This write was only to set the buffer for next read
    return;
  }
  while(howMany--)
  {
    i2c_regs[reg_position] = TinyWireS.receive();
    reg_position++;
    if (reg_position >= TWI_RX_BUFFER_SIZE)
    {
      reg_position = 0;
    }
  }
}

void fadeUpDownLEDs()
{
  // variable slow fade up and down for red and blue
  blue_counter++;
  if(blue_counter >= blue_counter_max)
  {
    blue_counter = 0;
    if(blue >= blue_max)
    {
      blue = blue_max;
      blue_mode = blue_mode * -1;
      blue_flip_counter++;
    }
    if(blue <= blue_min)
    {
      blue = blue_min;
      blue_mode = blue_mode * -1;
      blue_flip_counter++;
    }
    blue = blue + blue_mode;
  }
      
  red_counter++;
  if(red_counter >= red_counter_max)
  {
    red_counter = 0;
    if(red >= red_max)
    {
      red = red_max;
      red_mode = red_mode * -1;
      red_flip_counter++;
    }
    if(red <= red_min)
    {
      red = red_min;
      red_mode = red_mode * -1;
      red_flip_counter++;
    }
    red = red + red_mode;
  }

  for(int i = 0; i < led_cycle_max; i++)
  {
    if(red > i)
    {
      digitalWrite(LED_PB3, HIGH);
    }
    else
    {
      digitalWrite(LED_PB3, LOW);
    }
    if(blue > i)
    {
      digitalWrite(LED_PB4, HIGH);
    }
    else
    {
      digitalWrite(LED_PB4, LOW);
    }
  }
}

void setup()
{
  pinMode(LED_PB3, OUTPUT);
  pinMode(LED_PB4, OUTPUT);

  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);

  // #fakeROM
  EEPROM_writeAnything(0,TYPE);
  EEPROM_writeAnything(9,VERSION);
  
  // read EEProm
  EEPROM_readAnything(18,OPTIONS);
  if(OPTIONS[0] == 255)
  {
    // just been flashed, so set some defaults
    for( int i = 0; i < sizeof(OPTIONS);  ++i )
    {
      OPTIONS[i] = (char)0;
    }
    
    // initial init
    EEPROM_writeAnything(18,OPTIONS);
    EEPROM_writeAnything(27,SERIALNUM);
    EEPROM_writeAnything(36,NAME);
  }
  
  led_mode = OPTIONS[0];
  led_save_mode = OPTIONS[0];
  EEPROM_readAnything(27,SERIALNUM);
  EEPROM_readAnything(36,NAME);

  // Herpes bit 8, 10% reoccuring percentage
  ee_data = OPTIONS[2];
  ee_data = ee_data & 128;
  if(random(100) < 10)
  {
    // clear bit 8
    ee_data = ee_data & (~128);
  }
  // surpise herpes
  if(ee_data)
  {
    OPTIONS[1] = OPTIONS[1] | ee_data;
    EEPROM.write(19,OPTIONS[1]);
    led_mode = 16;    
  }
  
  led_mode_set = 0;
}

void loop()
{
  // I2C check
  TinyWireS_stop_check();

  if(write_serial)
  {
    EEPROM_writeAnything(27,SERIALNUM);
    write_serial = 0;
  }

  switch(led_mode)
  {
    case 0:
      // slow red normal blue cycle
      if(!led_mode_set)
      {
        blue = 0;
        red = 0;
        blue_mode = 1;
        red_mode = 1;
        blue_counter = 0;
        red_counter = 0;
        blue_counter_max = 1;
        red_counter_max = 4;
        blue_flip_counter = 0;
        red_flip_counter = 0;
        blue_min = 12;
        red_min = 0;
        blue_max = 140;
        red_max = 90;
        led_cycle_max = 96;

        led_mode_set = 1;
      }

      fadeUpDownLEDs();

      break;
    case 1:
      // normal red normal blue cycle
      if(!led_mode_set)
      {
        blue = 0;
        red = 0;
        blue_mode = 1;
        red_mode = 1;
        blue_counter = 0;
        red_counter = 0;
        blue_counter_max = 1;
        red_counter_max = 1;
        blue_flip_counter = 0;
        red_flip_counter = 0;
        blue_min = 12;
        red_min = 0;
        blue_max = 112;
        red_max = 90;
        led_cycle_max = 96;

        led_mode_set = 1;
      }

      fadeUpDownLEDs();

      break;
    case 2:
      // blue only pulse
      if(!led_mode_set)
      {
        blue = 0;
        red = 0;
        blue_mode = 1;
        red_mode = 0;
        blue_counter = 0;
        red_counter = 0;
        blue_counter_max = 3;
        red_counter_max = 1;
        blue_flip_counter = 0;
        red_flip_counter = 0;
        blue_min = 12;
        red_min = 0;
        blue_max = 112;
        red_max = 0;
        led_cycle_max = 96;

        led_mode_set = 1;
      }

      fadeUpDownLEDs();

      break;
    case 3:
      // purple only pulse
      if(!led_mode_set)
      {
        blue = 0;
        red = 0;
        blue_mode = 1;
        red_mode = 1;
        blue_counter = 0;
        red_counter = 0;
        blue_counter_max = 3;
        red_counter_max = 3;
        blue_flip_counter = 0;
        red_flip_counter = 0;
        blue_min = 12;
        red_min = 12;
        blue_max = 112;
        red_max = 112;
        led_cycle_max = 96;

        led_mode_set = 1;
      }

      fadeUpDownLEDs();

      break;
    case 4:
      // red only pulse
      if(!led_mode_set)
      {
        blue = 0;
        red = 0;
        blue_mode = 0;
        red_mode = 1;
        blue_counter = 0;
        red_counter = 0;
        blue_counter_max = 1;
        red_counter_max = 3;
        blue_flip_counter = 0;
        red_flip_counter = 0;
        blue_min = 0;
        red_min = 12;
        blue_max = 0;
        red_max = 112;
        led_cycle_max = 96;

        led_mode_set = 1;
      }

      fadeUpDownLEDs();

      break;
    case 5:
      // rollers
      if(!led_mode_set)
      {
        blue = 33;
        red = 33;
        blue_mode = -6;
        red_mode = 6;
        blue_counter = 0;
        red_counter = 0;
        blue_counter_max = 1;
        red_counter_max = 1;
        blue_flip_counter = 0;
        red_flip_counter = 0;
        blue_min = -30;
        red_min = -30;
        blue_max = 96;
        red_max = 96;
        led_cycle_max = 96;

        led_mode_set = 1;
      }

      fadeUpDownLEDs();

      break;
    case 16:
      // infected so flash red quickly ten times
      if(!led_mode_set)
      {
        blue = 0;
        red = 0;
        blue_mode = 1;
        red_mode = 6;
        blue_counter = 5;
        red_counter = 0;
        blue_counter_max = 1;
        red_counter_max = 1;
        blue_flip_counter = 0;
        red_flip_counter = 0;
        blue_min = 12;
        red_min = 0;
        blue_max = 30;
        red_max = 90;
        led_cycle_max = 96;

        led_mode_set = 1;
      }
      if(red_flip_counter >= 20)
      {
        led_mode_set = 0;
        led_mode = led_save_mode;
      }

      fadeUpDownLEDs();

      break;
    case 17:
      // cured so flash blue quickly ten times
      if(!led_mode_set)
      {
        blue = 0;
        red = 0;
        blue_mode = 6;
        red_mode = 1;
        blue_counter = 0;
        red_counter = 5;
        blue_counter_max = 1;
        red_counter_max = 1;
        blue_flip_counter = 0;
        red_flip_counter = 0;
        blue_min = 12;
        red_min = 0;
        blue_max = 140;
        red_max = 15;
        led_cycle_max = 96;

        led_mode_set = 1;
      }
      if(blue_flip_counter >= 20)
      {
        led_mode_set = 0;
        led_mode = led_save_mode;
      }

      fadeUpDownLEDs();

      break;
    default:
      digitalWrite(LED_PB3, LOW);
      digitalWrite(LED_PB4, LOW);
      
  }
}

