#include <Arduino.h>
#include <NMEAGPS.h>
#include "GPSPort.h"
#include <RHMesh.h>
#include <RH_RF69.h>
#include <SPI.h>

#define LED_RED 10
#define LED_GREEN 12


#define CHURCH_NODE 1 //The base node at camp ΒΣLLIGΣRΣΠCΣ
#define BIKE_NODE 2 //Which network node are we? 2..n for bikes 1..n-1
#undef BIKE_NODE
#define BELLIGERENCE F("ΒΣLLIGΣRΣΠCΣ") //handy string to send


typedef struct bikedata {
  NeoGPS::clock_t epochtime;
  int32_t latitude;
  int32_t longitude;
} bike_data;

// Mesh has much greater memory requirements, and you may need to limit the
// max message length to prevent wierd crashes
#define RH_MESH_MAX_MESSAGE_LEN sizeof(bikedata)

static bike_data bikeData;

// Singleton instance of the radio driver
RH_RF69 driver(8, 7); // Adafruit Feather 32u4

// Class to manage message delivery and receipt, using the driver declared above
RHMesh manager(driver, CHURCH_NODE);


// Dont put this on the stack:
uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];


void setup() {
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM

  // If you are using a high power RF69, you *must* set a Tx power in the
  // range 14 to 20 like this:
  driver.setTxPower(20);
  driver.setModemConfig(RH_RF69::GFSK_Rb2Fd5 );
   digitalWrite(LED_GREEN, LOW);
   digitalWrite(LED_RED, HIGH);
}

int packetCounter = 0;

void toggleState()
{
  static bool ledState = true;
  if (ledState) 
  {
   digitalWrite(LED_GREEN, LOW);
   digitalWrite(LED_RED, HIGH);
  }
  else
  {
     digitalWrite(LED_GREEN, HIGH);
     digitalWrite(LED_RED, LOW);
  }
  ledState = !ledState;
  
}

void blinkRed()
{
  //turn green off
  
}

void loop() {
  // put your main code here, to run repeatedly:

  static uint8_t len = sizeof(buf);
  static uint8_t from;
  static bike_data bikeMessage;
  toggleState();
  if (true)//manager.available())
  {
    // Serial.println(packetCounter);
    // Wait for a message addressed to us from the client
 
    if (manager.recvfromAck((uint8_t * ) &bikeData, &len, &from))
    {
      if (len != sizeof(bikedata) )
      {
        Serial.print("Warning: message length expected: ");
        Serial.print(sizeof(bikeData));
        Serial.print(" recieved: ");
        Serial.print(len);
      }
      Serial.print("message# ");
      Serial.print(packetCounter);
      Serial.print(". got message from : 0x");
      Serial.print(from, HEX);
      
      Serial.print(" time : ");
      Serial.print(bikeData.epochtime);
      Serial.print(" lat: ");
      Serial.print(bikeData.latitude);
      Serial.print(" lon: ");
      Serial.print(bikeData.longitude);
      Serial.println();
     

      // Send a reply back to the originator client
      //if (!manager.sendtoWait(data, sizeof(data), from))
      //  Serial.println("sendtoWait failed");
      packetCounter++;
    }
  }
  

}
