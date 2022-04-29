#ifndef MY_CONFIG_h
#define MY_CONFIG_h

const String unitinfo = "testxtray3";

// Pinouts
#define REDLED 12
#define GREENLED 13
#define YELLOWLED 11
#define BUZZER 10

//time to wait for Serial in ms
const uint16_t wait_for_Serial = 15000;

#define FLASH_DEBUG 0 // Use 0-2. Larger for more debugging messages


//#define FOUR_HX711 //comment to have only one HX711


// #define DEBUG // uncomment if not debugging
// #ifdef DEBUG
// #define debug(x) Serial.print(x)
// #define debugln(x) Serial.println(x)
// #else
// #define debug(x)
// #define debugln(x)
// #endif


#endif