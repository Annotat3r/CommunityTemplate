#include <cstring>
#include "NavRadio737.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include <Wire.h>


// ---HARDWARE DEFINITION FOR DISPLAY---
U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2(U8G2_R2, U8X8_PIN_NONE);

//----------------------------------------------------------------------
NavRadio737::NavRadio737()
{
    _initialised = false;
}

void NavRadio737::begin()
{
    // ---DEFINE I2C GPIO PINS FOR THE PICO---
  Wire.setSDA(4);
  Wire.setSCL(5);

    // ---START THE SCREEN---
  Wire.begin();
  Wire.setClock(400000); //Set I2C clock speed to 400khz (default is 100khz)

  u8g2.setI2CAddress(0x3C * 2); //I2C address of device
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.setFont(Font_NavRadio);

  for(int i=0; i<5; i++)
  {
    nav1.stbyDigits[i] = 0;
  }

  nav1.cursorPosition = 5;
  nav1.electricState = false;
  nav1.modeError = false;
}

void NavRadio737::attach()
{
}

void NavRadio737::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
}

void NavRadio737::set(int16_t messageID, char *setPoint)
{
    int32_t  data = atoi(setPoint);

    switch (messageID) {
    case -1:
        // ---SLEEP---
        nav1.electricState = (bool)data;
        break;
    case -2:
        // ---WHEN MF CONNECTOR IS STOPPED---
        nav1.electricState = (bool)data;
        break;
    case 0:
        // ---DELIVER ACTIVE FREQUENCY VALUE FROM MOBI---
        nav1.activeFrequency = (uint32_t)data; //Set the active frequency variable on device to value from MobiFlight
        break;
    case 1:
        // ---DELIVER ACTIVE MODE VALUE FROM MOBI---
        nav1.activeMode = (uint8_t)data; //Set active mode variable from MF value
        break;
    case 2:
        // ---DELIVER STANDBY MODE VALUE FROM MOBI---
        nav1.standbyMode = (uint8_t)data;
        break;
    case 3:
        // ---DELIVER STANDBY FREQUENCY ONES DIGIT FROM MOBI---
        nav1.stbyDigits[4] = (uint8_t)data;
        break;
    case 4:
        // ---DELIVER STANDBY FREQUENCY TENS DIGIT FROM MOBI---
        nav1.stbyDigits[3] = (uint8_t)data;
        break;
    case 5:
        // ---DELIVER STANDBY FREQUENCY HUNDREDS DIGIT FROM MOBI---
        nav1.stbyDigits[2] = (uint8_t)data;
        break;
    case 6:
        // ---DELIVER STANDBY FREQUENCY THOUSANDS DIGIT FROM MOBI---
        nav1.stbyDigits[1] = (uint8_t)data;
        break;
    case 7:
        // ---DELIVER STANDBY FREQUENCY TEN-THOUSANDS DIGIT FROM MOBI---
        nav1.stbyDigits[0] = (uint8_t)data;
        break;
    case 8:
        // ---DELIVER STANDBY FREQUENCY CURSOR POSITION FROM MOBI---
        nav1.cursorPosition = (uint8_t)data;
        break;
    case 9:
        // ---STANDBY FREQUENCY ENTRY ERROR---
        nav1.modeError = (bool)data;
        break;
    default:
        break;
    }

    if(nav1.electricState > 0)
    {
        buildStandbyString(&nav1);
        drawRadioScreen(&nav1);
    }
    else
    {
        shutDown();
    }
}

    // ---BUILD THE STANDBY FREQUENCY STRING---
void NavRadio737::buildStandbyString(NavRadio* r)
{
    bool hasDecimal = (r->standbyMode == 0 || r->standbyMode == 1);

    if(hasDecimal)
    {
        strcpy(r->standbyFreq, "___.__");
    }
    else
    {
        strcpy(r->standbyFreq, "___:__");
    }

    int digitsTyped = 5 - r->cursorPosition;
    if (digitsTyped < 0) digitsTyped = 0;

    int writeIdx = 0;
    for (int readIdx = 0; readIdx < digitsTyped; readIdx++)
    {
        //Skip the decimal at index 3, if the decimal exists
        if (writeIdx == 3)    //changed from: if (hasDecimal && writeIdx == 3)
        {
            writeIdx++;
        }
        r->standbyFreq[writeIdx] = r->stbyDigits[readIdx] + '0';
        writeIdx++;
    }
}

    // ---UPDATE THE SCREEN---
void NavRadio737::drawRadioScreen(NavRadio* r)
{
    u8g2.clearBuffer();
    u8g2.setFont(Font_NavRadio);

    //---DRAW THE ACTIVE MODE---
    u8g2.drawStr(0, 20, modeNames[r->activeMode]);	//draw the mode, reference modeNames array
        
    // ---DRAW THE ACTIVE FREQUENCY, INCLUDE DECIMAL IF REQUIRED BY MODE---
    if (r->activeMode == 0 || r->activeMode == 1) { 	// VOR or ILS Mode
	u8g2.setCursor(63, 23);
        u8g2.print(r->activeFrequency / 100);           // Prints the first 3 digits (e.g., 110)
        u8g2.print(".");                                // Print the decimal
        if (r->activeFrequency % 100 < 10) {            // Force a leading 0 if value of last 2 digits is less than 10
            u8g2.print("0");
        }
        u8g2.print(r->activeFrequency % 100);           // Prints the last 2 digits (e.g., 30)
    } else {                                            // GLS or LPV Modes (do not have a decimal)
	u8g2.setCursor(63, 23);
        u8g2.print(r->activeFrequency / 100);           // Print the first 3 digits
        u8g2.print(":");                                // Empty character to keep spacing format
        if (r->activeFrequency % 100 < 10) {            // Force a leading 0 if value of last 2 digits is less than 10
            u8g2.print("0");
        }
        u8g2.print(r->activeFrequency % 100);           // Print the last 2 digits
    }

    // ---DRAW THE STANDBY MODE---
    if(r->modeError)
    {
        u8g2.drawStr(0, 54, "ERR");
    }
    else
    {
        u8g2.drawStr(0, 54, modeNames[r->standbyMode]);
    }
    
    // ---DRAW THE STANDBY FREQUENCY---
    u8g2.setCursor(63, 56);
    u8g2.print(r->standbyFreq);

    // ---SEND IT TO THE DISPLAY---
    u8g2.sendBuffer();

}

void NavRadio737::shutDown()
{
    u8g2.clear();
}

void NavRadio737::update()
{
}
