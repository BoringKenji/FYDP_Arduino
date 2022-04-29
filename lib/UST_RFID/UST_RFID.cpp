#include "UST_RFID.h"

void rfidInit()
{
  const byte setPower[] = {0xBB, 0x00, 0xB6, 0x00, 0x02, 0x0A, 0x28, 0xEA, 0x7E}; // 26dBm
  // const byte setPower[] = {0xBB, 0x00, 0xB6, 0x00, 0x02, 0x07, 0xD0, 0x8F, 0x7E}; //20dBm
  // const byte setPower[] = {0xBB, 0x00, 0xB6, 0x00, 0x02, 0x07, 0x3A, 0xF9, 0x7E}; //18.5dBm
  //  const byte setPower[] = {0xBB, 0x00, 0xB6, 0x00, 0x02, 0x06, 0x0E, 0xCC, 0x7E}; //15.5dBm
  Serial1.write(setPower, sizeof(setPower));
  // Serial.println("set power to 26 dBm");
}


String multiscan_v2(uint8_t* numberOfTags, unsigned long timeout)
{
  // const byte readMulti[] = {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E}; //x1
  // const byte readMulti[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x00, 0x03, 0x4F, 0x7E}; // x3
  // const byte readMulti[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x00, 0x14, 0x60, 0x7E}; // x20
  const byte readMulti[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x00, 0x1E, 0x6A, 0x7E}; // x30
  bool finished_scan = false;
  uint8_t tags = 0;
  uint16_t i = 0;
  unsigned long currentMillis = 0; // millis() returns an unsigned long.
  Serial1.write(readMulti, sizeof(readMulti));
  currentMillis = millis(); // reset timer;
  uint8_t result[100] ={0};
  String finalTags = "";
  
  while (!finished_scan)
  {
    while (Serial1.available())
    {

      result[i] = Serial1.read();
      // Serial.print(result[i], HEX);
      // Serial.print(",");

      if (result[i] == 0x7e)
      {
        // Serial.println((String)"i: " + i);
        //if (i < 8)
        if ((i == 23) == (i == 19))
        { // if not enough characters, skipp this tag
          i = 0;
          // Serial.println("Skipped tag");
          continue;
        }
        tags++;
        i = 0;

        // Serial.println("do some data processing");

        
        String tagID = getTagfromHEX(result, 1);
        // Serial.println((String) "tagID: " + tagID);

        *numberOfTags = finalTags.length() / 8;
        // Serial.println((String) "numberOfTags: " + *numberOfTags);
        finalTags = keepIndividuals(finalTags, *numberOfTags, tagID, 1);
        // Serial.println((String) "finalTags: " + finalTags);

        continue;
      }
      i++;
      
      currentMillis = millis(); // reset timer;
    }

    if (millis() - currentMillis > timeout)
    { // if timeout is reached then finish reading
      // Serial.println((String) "numberOfTags: " + numberOfTags);
      // Serial.println((String) "finalTags: " + finalTags);

      // send to GUI
      // Serial.println((String) "n" + *numberOfTags); // send numberOfTags to GUI
      // Serial.println((String) "f" + finalTags);    // send finalTags to GUI
      
      //Serial.println.println("finished Read");

      finished_scan = true;
    }
  }
  return finalTags;
}

uint8_t multiscan(uint8_t result[], unsigned long timeout)
{
  // returns the number of tags
  // resutl ... the resulting scan as result array in HEX
  // timeout ... max time to wait after each Serial read
  // const byte readMulti[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x00, 0x0A, 0x56, 0x7E}; //x10
  // const byte readMulti[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x00, 0x0F, 0x5B, 0x7E}; //x15
  const byte readMulti[] = {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x00, 0x14, 0x60, 0x7E}; // x20
  bool finished_scan = false;
  uint8_t tags = 0;
  uint16_t i = 0;
  unsigned long currentMillis = 0; // millis() returns an unsigned long.
  Serial1.write(readMulti, sizeof(readMulti));
  currentMillis = millis(); // reset timer;

  while (!finished_scan)
  {
    while (Serial1.available())
    {
      result[i] = Serial1.read();
      // Serial.print(result[i],HEX);
      // Serial.print(",");

      if (result[i] == 0x7e)
      {
        // Serial.print(" ");
        tags++;
      }
      i++;
      currentMillis = millis(); // reset timer;
    }

    if (millis() - currentMillis > timeout)
    { // if timeout is reached then finish reading

      finished_scan = true;
      return tags;
    }
  }
  return 0;
}

char *getTagfromHEX_check(uint8_t HexArr[])
{
  static char result[2 * NUMBEROFBYTES + 1]; // Note, there needs to be 1 extra space for this to work as snprintf null terminates.
  char *myPtr = &result[0];
  for (uint8_t i = 0; i < 2; i++)
  {                                            // loop the tag id
    snprintf(myPtr, 3, "%02x", HexArr[5 + i]); // start from 12th to 16th and add 20 per tag scanned //convert a byte to character string, and save 2 characters (+null) to charArr;
    myPtr += 2;                                // increment the pointer by two characters in charArr so that next time the null from the previous go is overwritten.
  }

  return result;
}

String getTagfromHEX(uint8_t HexArr[], uint8_t nTags)
{
  // returns only the Tag numbers of interset
  static char result[2 * NUMBEROFBYTES + 1] = {0}; // Note there needs to be 1 extra space for this to work as snprintf null terminates.
  char *myPtr = &result[0];
  uint16_t j = 0;
  uint8_t tags = 0;
  // Serial.println("debugger");
  while (j < NUMBEROFBYTES && tags < nTags)
  { // loop through the whole HexArr till all Tags are checked
    if (HexArr[j] == 0x7e)
    {
      tags++;
      // Serial.println((String)"tags: " + tags);
      j = j - 7;
      // Serial.println((String)"j: " + j);
      for (uint8_t i = 0; i < 4; i++)
      { // loop the tag id
        // Serial.println((String)"HexArr: " + HexArr[j]);
        snprintf(myPtr, 3, "%02x", HexArr[j]); // start from 12th to 16th and add 20 per tag scanned //convert a byte to character string, and save 2 characters (+null) to charArr;
        myPtr += 2;                            // increment the pointer by two characters in charArr so that next time the null from the previous go is overwritten.
        j++;
      }
      j += 7;
      // Serial.println((String)"j: " + j);
    }
    j++;
  }
  // Serial.println((String)"result: " + result);
  return (String)result;
}

String keepIndividuals(String scan1, uint8_t nTags1, String scan2, uint8_t nTags2)
{
  uint8_t i = 0, j = 0;
  for (j = 0; j < nTags2; j++)
  {
    bool alreadyHere = false;
    if (scan2.substring(8 * j, 8 + 8 * j) != "bb01ff00")
    {
      for (i = 0; i < nTags1; i++)
      {
        if (scan1.substring(8 * i, 8 + 8 * i) == scan2.substring(8 * j, 8 + 8 * j))
        {
          alreadyHere = true;
          break;
        }
      }
      if (!alreadyHere)
      {
        scan1 += scan2.substring(8 * j, 8 + 8 * j);
      }
    }
  }
  return scan1;
}

String keepIndividualsOfMultiScan(String scan, uint8_t *ntotalTags)
{
  uint8_t j = 0, nTags = 0;
  String result = "";
  // Serial.println((String)"result: " + result);
  for (uint8_t i = 0; i < *ntotalTags; i++)
  {
    if (scan.substring(8 * i, 8 + 8 * i) != "bb01ff00")
    { // do the checking only if there is a valid tag
      j = 0;
      bool alreadyHere = false;
      while (j < nTags)
      {
        // Serial.println((String)"Compare: " + scan.substring(8 * i ,8 + 8 * i) + "with: " + result.substring(8 * j ,8 + 8 * j));
        if (scan.substring(8 * i, 8 + 8 * i) == result.substring(8 * j, 8 + 8 * j))
        {
          alreadyHere = true;
          break;
        }
        j++;
      }
      if (!alreadyHere)
      {
        result += scan.substring(8 * i, 8 + 8 * i); // Add ID to result
        nTags++;
      }
    }
  }
  *ntotalTags = nTags;
  return result;
}


String tagsChanged(String scan1, uint8_t nTags1, String scan2, uint8_t nTags2)
{
  String result = "";
  uint8_t i = 0, j = 0;
  for (j = 0; j < nTags2; j++)
  {
    bool alreadyHere = false;
    if (scan2.substring(8 * j, 8 + 8 * j) != "bb01ff00")
    {
      for (i = 0; i < nTags1; i++)
      {
        if (scan1.substring(8 * i, 8 + 8 * i) == scan2.substring(8 * j, 8 + 8 * j))
        {
          //Serial.println((String)"alreadyHere");
          alreadyHere = true;
          break;
        }
      }
      if (!alreadyHere)
      {
        result += scan2.substring(8 * j, 8 + 8 * j);
      }
    }
  }
  return result;
}