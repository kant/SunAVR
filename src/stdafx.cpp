#include "stdafx.h"
#include "LCD.h"
#include "RTC.h"
#include "RGBLED.h"
#include "SYSTEM.h"

// RTC
uint8_t RTC::rtc_bcd[8];

// LED
uint8_t LED::mLevel;
uint8_t LED::mCounter;
LED::Color LED::mColor(0xFF,0xFF,0xFF);

// SYSTEM
volatile uint8_t SYSTEM::mRunCmd;

/* ----------------------- hardware I/O abstraction ------------------------ */
// PIN FUNCTION
// PD0 LED BLUE (P3)
// PD1 LED GREEN (P2)
// PD2 LED RED (P1)
//~PD3 IR CMD INPUT
// PD4 LCD RS
//~PD5 LCD EN
//~PD6
// PD7
// PB0
//~PB1
//~PB2
//~PB3
// PB4
// PB5
// PC0 LCD D0
// PC1 LCD D1
// PC2 LCD D2
// PC3 LCD D3
// PC4 I2C SDA
// PC5 I2C SCL

//////////////////////////////////////////////////////////////////////////
