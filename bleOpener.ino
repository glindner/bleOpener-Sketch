/*

Copyright (c) 2014 George Lindner

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/* REVISION HISTORY
  March 28, 2014 - GTL - initial version 1.0 release
  
*/

/*

Services supported by this Sketch are limited to the following:

data0 = 0x01 is an incoming action request message where
  - data1 and data2 bytes are ignored
  - If the SEC_CODE is "0000" then the relay pin, DIGITAL_OUT_PIN, is operated (HIGH, pause, LOW)
  - Response message sets data1 to 0x01 for success and 0x00 for failure
    - failure response condition is a security code is needed

data0 = 0x02 is an incoming action request message where
  - data1 and data2 bytes contain the 4-digit security code entered by the App
    - data1 will contain the HIGH two digits (ie. XXxx where XX are the two digits)
    - data2 will contain the LOW two digits (ie. xxXX where XX are the two digits)
  - if the data1/2 digits match the SEC_CODE, then the relay pin, DIGITAL_PIN_OUT, is operated (HIGH, pause, LOW)
  - Response message sets data1 to 0x01 for success and 0x00 for failure
    - failure response condition is an invalid security code was received

data0 = 0x10 is an incoming settings request message where
  - data1 and data2 are ignored
  - Response message sets data1
    - 0x00 if SEC_CODE is not used (equal to "0000")
    - 0x01 if there is a SEC_CODE value present

*/

//"services.h/spi.h/boards.h" is needed in every new project
#include <SPI.h>
#include <boards.h>
#include <ble_shield.h>
#include <services.h>
 
#define DIGITAL_OUT_PIN    4
#define SEC_CODE "0000"  //must be 4-digits in length! "0000" disables security code checking

void setup()
{
  // Init. and start BLE library.
  ble_begin();
  
  // Enable serial debug
  Serial.begin(57600);
  
  pinMode(DIGITAL_OUT_PIN, OUTPUT);
}

void loop()
{
  
  // If data is ready
  while(ble_available())
  {
    // read out command and data
    byte data0 = ble_read();
    byte data1 = ble_read();
    byte data2 = ble_read();
    
    if (data0 == 0x01 && SEC_CODE == "0000")  // Command is to control digital out pin w/no security code defined
    {
      doRelay();
      data1 = 0x01; // success!
    }
    else if (data0 == 0x02) // Security code received, compare and control digital out pin
    {
      String secCode1 = String(data1);
      while (secCode1.length() < 2) secCode1 = "0" + secCode1; // pad to 2 digits
      String secCode2 = String(data2);
      while (secCode2.length() < 2) secCode2 = "0" + secCode2; // pad to 2 digits
      String secCode = secCode1 + secCode2; //string version of code
      if (secCode == SEC_CODE)
      {
        doRelay();
        data1 = 0x01; // success!
      }
      else data1 = 0x00; // fail!
    }
    else if (data0 == 0x10) // request if SEC_CODE feature enabled
    {
      // write 0x00 if disabled, 0x01 if enabled
      if (SEC_CODE == "0000") data1 = 0x00; else data1 = 0x01;
    }
    
    // send response...
    ble_write(data0);
    ble_write(data1);
    ble_write(0x00);  // future use

  }  
  if (!ble_connected())
  {
    digitalWrite(DIGITAL_OUT_PIN, LOW); //make sure relay is reset
  }
  
  // Allow BLE Shield to send/receive data
  ble_do_events();  
}

void doRelay()
{
  digitalWrite(DIGITAL_OUT_PIN, HIGH);  // perform relay close
  delay(500);  // wait a moment
  digitalWrite(DIGITAL_OUT_PIN, LOW);  // perform relay open
}

