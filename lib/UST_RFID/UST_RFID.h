#ifndef UST_RFID_h
#define UST_RFID_h

#include "Arduino.h"

#define TIMEOUT       900
#define NUMBEROFBYTES 7000 //max length for result array in getTagfromHEX() also in multiscan


//Commands
//const byte readSingle[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E};
//const byte readMulti[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x00, 0x03, 0x4F, 0x7E}; //x3
//const byte readMulti[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x00, 0x0A, 0x56, 0x7E}; //x10
//const byte stopRead[] = {0xBB, 0x00, 0x28, 0x00, 0x00, 0x28, 0x7E};
//const byte setBaudrate[] = {0xBB, 0x00, 0x11, 0x00, 0x02, 0x00, 0x60, 0x73, 0x7E};


//function prototypes
void rfidInit();
uint8_t multiscan(uint8_t result[], unsigned long timeout);
String getTagfromHEX(unsigned char HexArr[], uint8_t nTags);
char *getTagfromHEX_check(unsigned char HexArr[]);
String keepIndividuals(String scan1, uint8_t nTags1, String scan2, uint8_t nTags2);
String keepIndividualsOfMultiScan (String scan, uint8_t* nTags);
String multiscan_v2(uint8_t* numberOfTags, unsigned long timeout);
String tagsChanged(String scan1, uint8_t nTags1, String scan2, uint8_t nTags2);

#endif