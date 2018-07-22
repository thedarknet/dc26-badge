/**
 * Darknet "Official Eggplant Emoji Shitty Add-on (SAO) I2C Test code
 *
 * This is ugly and hacked together from an I2C example sketch.  
 * Just setup to test some of the I2C functions of the Eggplant SAO.
 * It's not optimal in any way.
 *
 */


int ee_address = 0;
byte i2c_regs[16] = {};
byte led_mode = 0;
byte imode = 1;

#include <Wire.h>

const String words[] = 
{
  "Testing",
  "Meatball",
  "Uber",
  "1234567890123456"
};

const String data[] = 
{
  "ABCEDEFG",
  "0123456789abcdef",
  "krux",
  "cmdc0de",
  "0a",
  "123456789012",
  "OICU812",
  "Krux Rules!"
};
byte data_index = 0;

const String names[] = 
{
  "Eggplant",
  "Eggie",
  "NotKrux",
  "cmdc0de",
  "0123456789abcdef",
  "OICU812",
};
byte names_index = 0;



void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
}

void loop()
{
  int cycle_delay = 3500;
  int c_mode = 0;
  byte addr_1 = 0;
  byte addr_2 = 0;
  byte addr_len = 0;
  
 /*
  
  // test setting names
  data_index++;
  if(data_index > 5)
  {
    data_index = 0;
  }
  addr_len = names[data_index].length();

  
  Wire.beginTransmission(0x36);
  // need to send an initial zero, otherwise we miss stuff
  Wire.write(0);
  // send noun, verb, data
  Wire.write(1);
  Wire.write(3);
  Wire.print(names[data_index]);
  Wire.write(0);
  Wire.endTransmission();
  
  Serial.print(1, DEC);
  Serial.print(" ");
  Serial.print(3, DEC);
  Serial.print(" ");
  Serial.print(names[data_index]);
  Serial.print("\n");
  delay(200);

  Wire.requestFrom(0x36, 16);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    if(c >= 32 && c <= 126)
    {
      Serial.print(c);
      Serial.print(" ");
    }
    else
    {
      Serial.print(c, DEC);         // print the character
      Serial.print(" ");
    }
  }
  Serial.print("\n");

  Serial.print("---\n\n");

  delay(cycle_delay);

  */

  /*
  addr_1 = 0;
  addr_2 = 1;
  
  // eeprom write test
  data_index++;
  if(data_index > 7)
  {
    data_index = 0;
  }
  addr_len = data[data_index].length();

  Serial.print(data[data_index]);
  Serial.print("\n");
  
  Wire.beginTransmission(0x36);
  // need to send an initial zero, otherwise we miss stuff
  Wire.write(0);
  // send noun, verb, data
  Wire.write(9);
  Wire.write(addr_1);
  Wire.write(addr_2);
  Wire.write(addr_len);
  Wire.print(data[data_index]);
  Wire.endTransmission();
  
  Serial.print(9, DEC);
  Serial.print(" ");
  Serial.print(addr_1, DEC);
  Serial.print(" ");
  Serial.print(addr_2, DEC);
  Serial.print(" ");
  Serial.print(addr_len, DEC);
  Serial.print("\n");
  delay(200);

  Wire.requestFrom(0x36, 16);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    if(c >= 32 && c <= 126)
    {
      Serial.print(c);
      Serial.print(" ");
    }
    else
    {
      Serial.print(c, DEC);         // print the character
      Serial.print(" ");
    }
  }
  Serial.print("\n");

  Serial.print("---\n\n");

  delay(cycle_delay);

*/

  // led mode test
  led_mode++;
  if(led_mode > 5)
  {
    led_mode = 0;
  }

  Wire.beginTransmission(0x36);
  // need to send an initial zero, otherwise we miss stuff
  Wire.write(0);
  // send noun, verb, data
  Wire.write(3);
  Wire.write(led_mode);
  Wire.endTransmission();
  
  Serial.print(3, DEC);
  Serial.print(" ");
  Serial.print(led_mode, DEC);
  Serial.print("\n");
  delay(200);

  Wire.requestFrom(0x36, 16);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    if(c >= 32 && c <= 126)
    {
      Serial.print(c);
      Serial.print(" ");
    }
    else
    {
      Serial.print(c, DEC);         // print the character
      Serial.print(" ");
    }
  }
  Serial.print("\n");

  Serial.print("---\n\n");

  delay(cycle_delay);

if(imode == 1)
{
  imode = 0;
  Wire.beginTransmission(0x36);
  // need to send an initial zero, otherwise we miss stuff
  Wire.write(0);
  // send noun, verb, data
  Wire.write(5);
  Wire.write(0);
  Wire.write(192);
  Wire.endTransmission();
  
  Serial.print(5, DEC);
  Serial.print(" ");
  Serial.print(0, DEC);
  Serial.print(" ");
  Serial.print(192, BIN);
  Serial.print("\n");
  delay(200);

  Wire.requestFrom(0x36, 16);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    if(c >= 32 && c <= 126)
    {
      Serial.print(c);
      Serial.print(" ");
    }
    else
    {
      Serial.print(c, DEC);         // print the character
      Serial.print(" ");
    }
  }
  Serial.print("\n");

  Serial.print("---\n\n");

  delay(cycle_delay);
}
else
{
  imode = 1;

  Wire.beginTransmission(0x36);
  // need to send an initial zero, otherwise we miss stuff
  Wire.write(0);
  // send noun, verb, data
  Wire.write(5);
  Wire.write(1);
  Wire.write(192);
  Wire.endTransmission();
  
  Serial.print(5, DEC);
  Serial.print(" ");
  Serial.print(1, DEC);
  Serial.print(" ");
  Serial.print(192, BIN);
  Serial.print("\n");
  delay(200);

  Wire.requestFrom(0x36, 16);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    if(c >= 32 && c <= 126)
    {
      Serial.print(c);
      Serial.print(" ");
    }
    else
    {
      Serial.print(c, DEC);         // print the character
      Serial.print(" ");
    }
  }
  Serial.print("\n");

  Serial.print("---\n\n");

  delay(cycle_delay);

}

  Wire.beginTransmission(0x36);
  // need to send an initial zero, otherwise we miss stuff
  Wire.write(0);
  // send noun, verb, data
  Wire.write(4);
  Wire.write(0);
  Wire.endTransmission();
  
  Serial.print(4, DEC);
  Serial.print(" ");
  Serial.print(0, DEC);
  Serial.print("\n");
  delay(200);

  Wire.requestFrom(0x36, 16);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    if(c >= 32 && c <= 126)
    {
      Serial.print(c);
      Serial.print(" ");
    }
    else
    {
      Serial.print(c, DEC);         // print the character
      Serial.print(" ");
    }
  }
  Serial.print("\n");

  Serial.print("---\n\n");

  delay(cycle_delay);

  Wire.beginTransmission(0x36);
  // need to send an initial zero, otherwise we miss stuff
  Wire.write(0);
  // send noun, verb, data
  Wire.write(4);
  Wire.write(1);
  Wire.endTransmission();
  
  Serial.print(4, DEC);
  Serial.print(" ");
  Serial.print(1, DEC);
  Serial.print("\n");
  delay(200);

  Wire.requestFrom(0x36, 16);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    if(c >= 32 && c <= 126)
    {
      Serial.print(c);
      Serial.print(" ");
    }
    else
    {
      Serial.print(c, DEC);         // print the character
      Serial.print(" ");
    }
  }
  Serial.print("\n");

  Serial.print("---\n\n");

  delay(cycle_delay);




  // eeprom read test
  
  addr_1 = 18;
  addr_2 = 0;
  addr_len = 16;
  
  Wire.beginTransmission(0x36);
  // need to send an initial zero, otherwise we miss stuff
  Wire.write(0);
  // send noun, verb, data
  Wire.write(8);
  Wire.write(addr_1);
  Wire.write(addr_2);
  Wire.write(addr_len);
  Wire.endTransmission();
    
  Serial.print(8, DEC);
  Serial.print(" ");
  Serial.print(addr_1, DEC);
  Serial.print(" ");
  Serial.print(addr_2, DEC);
  Serial.print(" ");
  Serial.print(addr_len, DEC);
  Serial.print("\n");
  delay(200);

  Wire.requestFrom(0x36, 16);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    if(c >= 32 && c <= 126)
    {
      Serial.print(c);
      Serial.print(" ");
    }
    else
    {
      Serial.print(c, DEC);         // print the character
      Serial.print(" ");
    }
  }
  Serial.print("\n");

  Serial.print("---\n\n");

  delay(cycle_delay);

  
  // system info and led mode test
  for(byte x = 0 ; x < 4; x = x + 2)
  {
    for(byte y = 0 ; y < 4; y++)
    {
      Wire.beginTransmission(0x36);
      // need to send an initial zero, otherwise we miss stuff
      Wire.write(0);
      // send noun, verb, data
      Wire.write(x);
      Wire.write(y);
      // Wire.write(0);
      // Wire.print(words[y]);
      Wire.endTransmission();
    
      Serial.print(x, DEC);
      Serial.print(" ");
      Serial.print(y, DEC);
      Serial.print(" 0\n");
      delay(200);

      Wire.requestFrom(0x36, 16);    // request 6 bytes from slave device #2

      c_mode = 255;
      while(Wire.available())    // slave may send less than requested
      { 
        char c = Wire.read(); // receive a byte as character
        if(c_mode == 255)
        {
          c_mode = c;
        }
        if(c >= 32 && c <= 126 && c_mode != 2)
        {
          Serial.print(c);
          Serial.print(" ");
        }
        else
        {
          Serial.print(c, DEC);         // print the character
          Serial.print(" ");
        }
      }
      Serial.print("\n");

      delay(500);
    }
    Serial.print("---\n\n");

    delay(cycle_delay);
  }
}
