#include <Arduino.h>

//Radio Module
#include <RHMesh.h>
#include <RH_RF69.h>
#include <SPI.h>

#define BIKE_NODE 6 //Which network node are we? 1..5 for bikes 1..5, 6..n for repeaters

// Singleton instance of the radio driver
RH_RF69 driver(8, 7); // Adafruit Feather 32u4

// Class to manage message delivery and receipt, using the driver declared above
RHMesh manager(driver, BIKE_NODE);

typedef struct bikedata {
  uint32_t epochtime;
  int32_t latitude;
  int32_t longitude;
  uint16_t batteryVoltage;
} bike_data;

// Mesh has much greater memory requirements, and you may need to limit the
// max message length to prevent wierd crashes
#define RH_MESH_MAX_MESSAGE_LEN sizeof(bikedata)

bike_data bikeData;


void setup() {
  manager.init();
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM

  // If you are using a high power RF69, you *must* set a Tx power in the
  // range 14 to 20 like this:
  driver.setTxPower(20, true); // true here means we have a high-power RFM69
  driver.setModemConfig(RH_RF69::GFSK_Rb2Fd5 );

  manager.setRetries(2);
  manager.setTimeout(245); // Minimum timeout, actual value [timeout,...,timeout*2]
}

// Dont put this on the stack:
static uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
static uint8_t len = sizeof(buf);

void loop() {
  manager.recvfromAck(buf, &len);//Forward messages
  
}
