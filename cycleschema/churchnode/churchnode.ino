
// Mesh has much greater memory requirements, and you may need to limit the
// max message length to prevent wierd crashes
#define RH_MESH_MAX_MESSAGE_LEN 50

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

// Singleton instance of the radio driver
RH_RF69 driver(8, 7); // Adafruit Feather 32u4

// Class to manage message delivery and receipt, using the driver declared above
RHMesh manager(driver, CHURCH_NODE);

// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];


void setup() {
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  
  Serial.begin(9600);
  while (!Serial) 
    ;
  Serial.println("Serial, Yo.");
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
bool ledState = true;

void toggleState()
{
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
void loop() {
  // put your main code here, to run repeatedly:
  toggleState();
  if (true)//manager.available())
  {
    // Serial.println(packetCounter);
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      Serial.print("message# ");
      Serial.print(packetCounter);
      Serial.print(". got request from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);

      // Send a reply back to the originator client
      //if (!manager.sendtoWait(data, sizeof(data), from))
      //  Serial.println("sendtoWait failed");
      packetCounter++;
    }
  }
  

}
