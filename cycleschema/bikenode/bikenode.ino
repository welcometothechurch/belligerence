
#include <NMEAGPS.h>
#include "GPSPort.h"
#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include <SPI.h>


#define LED_RED 10
#define LED_GREEN 12

#define CHURCH_NODE 1 //The base node at camp ΒΣLLIGΣRΣΠCΣ
#define BIKE_NODE 2 //Which network node are we? 2..n for bikes 1..n-1
#define BELLIGERENCE "ΒΣLLIGΣRΣΠCΣ" //handy string to send

// Singleton instance of the radio driver

RH_RF69 driver(8, 7); // Adafruit Feather 32u4

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, BIKE_NODE);


void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  if (!manager.init()) {
    digitalWrite(LED_RED, HIGH);
    delay(400);
    digitalWrite(LED_RED, LOW); 
    delay(100);
  }
   digitalWrite(LED_GREEN, LOW);
   digitalWrite(LED_RED, HIGH);
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM

  // If you are using a high power RF69, you *must* set a Tx power in the
  // range 14 to 20 like this:
  // driver.setTxPower(14);
}

uint8_t data[] = BELLIGERENCE;
// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
void loop() {
  digitalWrite(LED_GREEN, HIGH); 
  // Send a message to manager_server
  digitalWrite(LED_GREEN, HIGH);
  if (manager.sendtoWait(data, sizeof(data), CHURCH_NODE))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    /*
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
       
    }
    else
    {
        
    }
    */
  }
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW); 
  delay(300);
  digitalWrite(LED_RED, HIGH); 
 
}
