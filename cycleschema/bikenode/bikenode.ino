#include <Arduino.h>

//local GPS conf
#include <NeoHWSerial.h>
#include "NeoGPS_cfg.h"
#include "NMEAGPS_cfg.h"
#include "GPSFix_cfg.h"
#include <NMEAGPS.h>
#include <streamers.h>
#include <avr/wdt.h>

//Radio Module
#include <RHMesh.h>
#include <RH_RF69.h>
#include <SPI.h>


#define LED_RED 10
#define LED_GREEN 12

#define CHURCH_NODE 0 //The base node at camp ΒΣLLIGΣRΣΠCΣ
#define BIKE_NODE 5 //Which network node are we? 1..n for bikes 1..n
#define BELLIGERENCE "ΒΣLLIGΣRΣΠCΣ" //handy string to send
#define SPEED_THRESH 550 // (0.55 kn / ~1 km/h) Minimum speed to reach before we trigger a send, in milli-knots
#define HEARTBEAT 299000 // (4m59s) millis() between compulsory messages


// Singleton instance of the radio driver
RH_RF69 driver(8, 7); // Adafruit Feather 32u4

// Class to manage message delivery and receipt, using the driver declared above
RHMesh manager(driver, BIKE_NODE);

//------------------------------------------------------------
// This object parses received characters
//   into the gps.fix() data structure

static NMEAGPS  gps;

//------------------------------------------------------------
//  Define a set of GPS fix information.  It will
//  hold on to the various pieces as they are received from
//  an RMC sentence.  It can be used anywhere in your sketch.

static gps_fix  fix_data;

typedef struct bikedata {
  NeoGPS::clock_t epochtime;
  int32_t latitude;
  int32_t longitude;
  uint16_t batteryVoltage;
} bike_data;

// Mesh has much greater memory requirements, and you may need to limit the
// max message length to prevent wierd crashes
#define RH_MESH_MAX_MESSAGE_LEN sizeof(bikedata)

bike_data bikeData;

boolean sendData = false; // Flag that we need to send a packet

uint32_t hbMillis; // Time to send another packet as heartbeat

//Is it our turn to talk?
// Implement a simple time-division multiplexing thing so that our messages have a
// chance of navigating the low-speed mesh before another bike yells its coordinates.
inline bool myTurn(NeoGPS::clock_t *epochtime)
{

  return BIKE_NODE == ((*epochtime) % 5) + 1 ;
}

//Interrupt service routine for char-at-a-time update from the GPS string
static void GPSisr( uint8_t c )
{
  gps.handle( c );
  //toggleLEDs();
} // GPSisr

//Return our battery voltage as an unsigned integer, save those bytes!
uint16_t readBatteryVoltage()
{
#define VBATPIN A9

  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  measuredvbat *=  100; //scale to a nice integer
  return (uint16_t) measuredvbat; //cast to an integer
  //Serial.print("VBat: " ); Serial.println(measuredvbat);
}

inline void toggleLEDs()
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


void setup() {
  wdt_enable(WDTO_4S);
  //pinMode(LED_RED, OUTPUT); // LEDs disabled in prod to save battery
  //pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, HIGH);
  if (!manager.init()) {
    digitalWrite(LED_RED, LOW);
    delay(400);
    digitalWrite(LED_RED, HIGH);
    delay(100);
  }
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM

  // If you are using a high power RF69, you *must* set a Tx power in the
  // range 14 to 20 like this:
  driver.setTxPower(20, true); // true here means we have a high-power RFM69
  driver.setModemConfig(RH_RF69::GFSK_Rb4_8Fd9_6 );

  manager.setRetries(4); // Two tries at sending, but we need to be done and ready to forward other packets before our timeslot closes
  manager.setTimeout(99); // Minimum timeout, actual value [timeout,...,timeout*2]

  //Start the UART for the GPS device
  NeoSerial1.attachInterrupt( GPSisr );
  NeoSerial1.begin( 9600 );
  /*Serial.begin(115200);
    while (!Serial)

    ;
    Serial.println("Serial, Yo.");/**/
  hbMillis = millis();
  
  toggleLEDs();
  delay(100);
  toggleLEDs();
  delay(100);
  toggleLEDs();
  delay(100);
  toggleLEDs();
  delay(100);
  toggleLEDs();
  delay(100);
  toggleLEDs();
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  wdt_reset();
}

uint8_t data[] = BELLIGERENCE;
// Dont put this on the stack:
static uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
static uint8_t len = sizeof(buf);

static gps_fix         fix;

void loop() {
  //toggleLEDs();
  manager.recvfromAckTimeout(buf, &len, 10);//Forward any messages that aren't ours?
  while (gps.available()) {
    fix      = gps.read();
    //trace_all( Serial, gps, fix);
    if ( fix.valid.location)
    {
      bikeData.latitude = fix.latitudeL();
      bikeData.longitude = fix.longitudeL();
    }
    else
    {
      bikeData.latitude = 999;
      bikeData.longitude = 999;
      digitalWrite(LED_RED, LOW);
    }
    //get the battery voltage
    bikeData.batteryVoltage = readBatteryVoltage();

    if (fix.valid.time)
    {
      bikeData.epochtime = fix.dateTime;
    }
    else
    {
      bikeData.epochtime = 999;
      digitalWrite(LED_RED, LOW);
    }

    if (fix.valid.speed){
      if (fix.speed_mkn() > SPEED_THRESH) {
        sendData = true;
        digitalWrite(LED_GREEN, LOW);
      }
    }

    if (millis() > hbMillis) {
      sendData = true;
      digitalWrite(LED_GREEN, LOW);
    }
    
    //Serial.println("Blorp:");
    //Serial.write((uint8_t * ) &aFix, sizeof(aFix));
    //Serial.println(fix_data.latitude());

    if ( fix.valid.time && myTurn(&(bikeData.epochtime)) && sendData) //only send data if it's valid and its our turn to talk.
    {
      if (manager.sendtoWait((uint8_t * ) &bikeData, sizeof(bikeData), CHURCH_NODE) == RH_ROUTER_ERROR_NONE)
      {
        sendData = false;
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_RED, HIGH);
        hbMillis = millis() + HEARTBEAT;
      } else {
        digitalWrite(LED_RED, LOW);
      }

    }

  }
  wdt_reset();
}
