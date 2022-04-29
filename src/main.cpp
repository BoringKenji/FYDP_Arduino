#include <Arduino.h>

#include "myconfig.h"
#include <FlashStorage_SAMD.h>
#include <SPI.h>
#include <Wire.h>
#include "UST_RFID.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Firebase_Arduino_WiFiNINA.h>

#define DATABASE_URL "fypd-d0e2e-default-rtdb.asia-southeast1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "25s3f2zX5mzmmNgkvbg7yOajFX84Og1cNqVAlwvx"
#define WIFI_SSID "Kenjiwifi"
#define WIFI_PASSWORD "%07Lv651"

FirebaseData fbdo;

int forcePin0 = 0;
int forcePin1 = 1;
int forcePin2 = 2;
int forcePin3 = 3;
double forceReading0;
double forceReading1;
double forceReading2;
double forceReading3;

void waitSerial(uint16_t waittime);
void displaySavedData();
void eepromInit();
void oledInit();
void addtotags();

const int WRITTEN_SIGNATURE = 0xBEEFDEED;

typedef struct
{
  char SSID[30];
  char PWD[30];
  char SCALEFACTOR[10];
} EEPROM_data;

int signature;
uint16_t storedAddress = 0;
EEPROM_data e_data;

String data;
char d1;
String x;
float scalefactor;

// basic setting of hardware

// Timer
unsigned long myTime, scanTimeOut = 0;
// screen
//  On an arduino UNO:       A4(SDA), A5(SCL)
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET 4        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// #define timeout 900
// uint8_t totaltags[1000];

void setup()
{
  randomSeed(analogRead(0));

  Serial1.begin(115200); // for RFID
  Serial.begin(115200);
  // Serial.setTimeout(500);
  oledInit();

  waitSerial(wait_for_Serial);
  Serial.println("Serial port ready");
  eepromInit();

  rfidInit();
  Serial.println("RFID ready");

  // start to set up wifi and firebase connection.
  Serial.print("Connecting to Wi-Fi");
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED)
  {
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print(".");
    delay(100);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // Provide the autntication data
  Firebase.begin(DATABASE_URL, DATABASE_SECRET, WIFI_SSID, WIFI_PASSWORD);
  Firebase.reconnectWiFi(true);
  Serial.println("Setup ready!");
}

void loop()
{
  Serial.println();
  Serial.println();
  Serial.println("beginn of main loop");
  Serial.println();

  myTime = millis();

  // RFID
  // Scan RFID

  uint8_t numberOfTags = 0;
  static uint8_t prev_numberOfTags = 0;
  String finalTags = multiscan_v2(&numberOfTags, TIMEOUT);
  Serial.println((String) "n" + numberOfTags); // send numberOfTags to GUI
  Serial.println((String) "f" + finalTags);    // send finalTags to GUI

  static String prev_finalTags = "";
  // Serial.println((String)"finalTags: " + finalTags);
  //  Serial.println((String)"prev_finalTags: " + prev_finalTags);
  // Serial.println((String)"numberOfTags: " + numberOfTags);
  //  Serial.println((String)"prev_numberOfTags: " + prev_numberOfTags);
  String addedTags = tagsChanged(prev_finalTags, prev_numberOfTags, finalTags, numberOfTags);
  String removedTags = tagsChanged(finalTags, numberOfTags, prev_finalTags, prev_numberOfTags);
  // Serial.println((String)"added Tags: " + addedTags);
  // Serial.println((String)"removed Tags: " + removedTags);
  Serial.println((String) "d" + addedTags);   // send numberOfTags to GUI
  Serial.println((String) "r" + removedTags); // send finalTags to GUI

  prev_finalTags = finalTags;
  prev_numberOfTags = numberOfTags;

  // Copy from FYDP///////////////////////////////////////////////////////////////////////////////////////////////

  forceReading0 = analogRead(forcePin0);
  Serial.print("Analog reading of A0 = ");
  Serial.println(forceReading0);

  // Analog Pin 1
  forceReading1 = analogRead(forcePin1);
  Serial.print("Analog reading of A1 = ");
  Serial.println(forceReading1);

  // Analog Pin 2
  forceReading2 = analogRead(forcePin2);
  Serial.print("Analog reading of A2 = ");
  Serial.println(forceReading2);

  // Analog Pin 3
  forceReading3 = analogRead(forcePin3);
  Serial.print("Analog reading of A3 = ");
  Serial.println(forceReading3);

  // Total sum of weight
  Serial.println("--------------------------------------------");
  Serial.print("Analog in total:");
  double Totalreading = forceReading0 + forceReading1 + forceReading2 + forceReading3;
  Serial.println(Totalreading);
  Serial.print("Weight in total: (polynomial quation)");
  double Totalweight = 3 * 0.00000001 * Totalreading * Totalreading * Totalreading - 3 * 0.00001 * Totalreading * Totalreading + 0.0231 * Totalreading - 33.3;
  Serial.println(Totalweight);

  // Push weight data to firebase
  String path_weight = "/test";
  String jsonStr;
  String weight = String(Totalweight, 2);
  jsonStr = "{\"Weight_in_gram\":" + weight + ",\"RFID_tags_id_array\":\""+ finalTags +"\",\"Timestamp\":{\".sv\":\"timestamp\"}}";

  if (Firebase.pushJSON(fbdo, path_weight, jsonStr))
  {
    Serial.println("ok");
    Serial.println("path_weight: " + fbdo.dataPath());
    Serial.print("push name: ");
    Serial.println(fbdo.pushName());
  }
  else
  {
    Serial.println("error, " + fbdo.errorReason());
  }

  // Update chemicals data to firebase
  // finalTags = "02"; // delete this line when RFID reader working
  for (unsigned int i = 0; i < finalTags.length(); i = i + 8)
  {
    String target_id = finalTags.substring(i, i + 8);
    Serial.print("target_id = ");
    Serial.print(target_id);
    String path_chemicals = "/chemicals/" + target_id;
    String jsonStr_chemicals;
    jsonStr_chemicals = "{\"ID\": \"" + target_id + "\"}";
    if (Firebase.updateNode(fbdo, path_chemicals, jsonStr_chemicals))
    {
      // Database path_chemicals that updated
      Serial.println(fbdo.dataPath());
      // Data type at updated database path_chemicals
      Serial.println(fbdo.dataType()); // Should be "json"
      // Print the JSON string payload that returned from server
      Serial.println(fbdo.jsonData()); // Should mathes the value in updateData variable
      // Actual sent payload JSON data
      Serial.println(jsonStr_chemicals);
    }
    else
    {
      // Failed, then print out the error detail
      Serial.println(fbdo.errorReason());
    }
  }
  
  

  Serial.println("--------------------------------------------");

  delay(3000);
}

// private functions ******************************************************************

void waitSerial(uint16_t waittime)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print((String) "Wait " + waittime / 1000 + " s for Serial...");
  display.display();

  // waits for Serial Port or the wiattime has elapsed
  long prev_time = millis();
  long tim = 0;
  while (!Serial)
  {
    tim = millis() - prev_time;
    if (tim > waittime)
      break;
  };
}

void displaySavedData()
{
  Serial.print("W");
  Serial.println(e_data.SSID);
  Serial.print("F");
  Serial.println(e_data.SCALEFACTOR);
}

void eepromInit()
{ // safes default values to eeprom if empty
  EEPROM.get(storedAddress, signature);

  // If the EEPROM is empty then no WRITTEN_SIGNATURE
  if (signature == WRITTEN_SIGNATURE) // if something is in Flash
  {
    EEPROM.get(storedAddress + sizeof(signature), e_data);
    displaySavedData();
  }
  else // safe default settings to eeprom if empty
  {
    EEPROM.put(storedAddress, WRITTEN_SIGNATURE);

    String SCALEFACTOR = "-200";

    // Fill the "e_data" structure with the data entered by the user...

    SCALEFACTOR.toCharArray(e_data.SCALEFACTOR, 10);

    // ...and finally save everything into emulated-EEPROM
    EEPROM.put(storedAddress + sizeof(signature), e_data);

    if (!EEPROM.getCommitASAP())
    {
      Serial.println("CommitASAP not set. Need commit()");
      EEPROM.commit();
    }
  }
}

void oledInit()
{
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(2);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner
  display.println("Wesahre");
  display.println("...loading");
  display.display();
}