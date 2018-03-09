#include <Arduino.h>
#include <NMEAGPS.h>
#include "GPSPort.h"
#include <RHMesh.h>
#include <RH_RF69.h>
#include <SPI.h>

#define LED_RED 10
#define LED_GREEN 12


#define CHURCH_NODE 0 //The base node at camp ΒΣLLIGΣRΣΠCΣ
#define BELLIGERENCE F("ΒΣLLIGΣRΣΠCΣ") //handy string to send


typedef struct bikedata {
  NeoGPS::clock_t epochtime;
  int32_t latitude;
  int32_t longitude;
  uint16_t batteryVoltage;
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

//Separator character for our serial output
static char separator[] = ",";
#define PRINT_SEPARATOR Serial.print(separator);

//only slightly better than inlining all the print code :)
void emitBikeData(uint8_t *node,bike_data *bikeData)
{
  Serial.print(*node);
  PRINT_SEPARATOR
  Serial.print(bikeData->epochtime);
  PRINT_SEPARATOR
  Serial.print(bikeData->latitude);
  PRINT_SEPARATOR
  Serial.print(bikeData->longitude);
  PRINT_SEPARATOR
  Serial.print(bikeData->batteryVoltage);
  Serial.println();
  
   
}

void setup() {
  Serial.begin(9600);
// No LEDs on the church node
//  pinMode(LED_RED, OUTPUT);
//  pinMode(LED_GREEN, OUTPUT);
//
//  digitalWrite(LED_GREEN, HIGH);
//  digitalWrite(LED_RED, LOW);
//
//  delay(3000); // Wait for serial monitor
  
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM

  // If you are using a high power RF69, you *must* set a Tx power in the
  // range 14 to 20 like this:
  driver.setTxPower(20, true); // True here means we have a high-power RFM69
  driver.setModemConfig(RH_RF69::GFSK_Rb4_8Fd9_6 );

  manager.setRetries(4); // Two tries at sending, but we need to be done and ready to forward other packets before our timeslot closes
  manager.setTimeout(99); // Minimum timeout, actual value [timeout,...,timeout*2]
//   digitalWrite(LED_GREEN, LOW);
//   digitalWrite(LED_RED, HIGH);
//   Serial.println("init fin");
}

int packetCounter = 0;

//void toggleState()
//{
//  static bool ledState = true;
//  if (ledState) 
//  {
//   digitalWrite(LED_GREEN, LOW);
//   digitalWrite(LED_RED, HIGH);
//  }
//  else
//  {
//     digitalWrite(LED_GREEN, HIGH);
//     digitalWrite(LED_RED, LOW);
//  }
//  ledState = !ledState;
//  
//}


void loop() {
  // put your main code here, to run repeatedly:

  uint8_t len = sizeof(buf);
  uint8_t from;
  //static bike_data bikeMessage;
 // toggleState();
  if (true)//manager.available())
  {
    //Serial.println(packetCounter);
    // Wait for a message addressed to us from the client
 
    if (manager.recvfromAck((uint8_t * ) &bikeData, &len, &from))
    {
      if (len != sizeof(bikedata) )
      {
        Serial.print("Warning: message length expected: ");
        Serial.print(sizeof(bikeData));
        Serial.print(" recieved: ");
        Serial.print(len);
        Serial.println();
      }
      emitBikeData(&from,&bikeData);
      /*Serial.print("message# ");
      Serial.print(packetCounter);
      Serial.print(". got message from bike: ");
      Serial.print(from, DEC);
      
      Serial.print(" time : ");
      Serial.print(bikeData.epochtime);
      Serial.print(" lat: ");
      Serial.print(bikeData.latitude);
      Serial.print(" lon: ");
      Serial.print(bikeData.longitude);
      Serial.println();
     /**/
     

      // Send a reply back to the originator client
      //if (!manager.sendtoWait(data, sizeof(data), from))
      //  Serial.println("sendtoWait failed");
      packetCounter++;
    }
  }
  

}
