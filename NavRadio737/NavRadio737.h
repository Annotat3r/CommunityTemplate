#pragma once

#include "Arduino.h"
#include "Font/u8g2_Font_NavRadio.h"
#include "U8g2lib.h"

extern U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2;

struct NavRadio {
    uint8_t activeMode;         //Value 0-3
    uint8_t standbyMode;        //Value 0-3
    uint8_t stbyDigits[5];      //Array for storing raw digits 0-9
    uint8_t cursorPosition;     //Value 0-5
    uint32_t activeFrequency;   //Stored as integer (no decimal in this value)
    char standbyFreq[8];        //Array for standby frequency values
    bool electricState;
    bool modeError;
};

inline NavRadio nav1;

inline const char* modeNames[] = {
    "VOR",  //Index 0
    "ILS",  //Index 1
    "GLS",  //Index 2
    "LPV"   //Index 3
};


class NavRadio737
{
public:
    NavRadio737();
    void begin();
    void attach();
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();
    void updateActiveFrequency();
    void shutDown();

private:
    bool    _initialised;
    void drawRadioScreen(NavRadio* r);
    void buildStandbyString(NavRadio* r);
};