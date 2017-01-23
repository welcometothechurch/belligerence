#include <Arduino.h>

//local GPS conf
#include <NeoHWSerial.h>
#include "NeoGPS_cfg.h"
#include "NMEAGPS_cfg.h"
#include "GPSFix_cfg.h"
#include <NMEAGPS.h>
#include <streamers.h>

//Radio Module
#include <RHMesh.h>
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
} bike_data;

// Mesh has much greater memory requirements, and you may need to limit the
// max message length to prevent wierd crashes
#define RH_MESH_MAX_MESSAGE_LEN sizeof(bikedata)

bike_data bikeData;


//Interrupt service routine for char-at-a-time update from the GPS string
static void GPSisr( uint8_t c )
{
  gps.handle( c );
} // GPSisr



void toggleLEDs()
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
  driver.setTxPower(20);
  driver.setModemConfig(RH_RF69::GFSK_Rb2Fd5 );


  //Start the UART for the GPS device
  NeoSerial1.attachInterrupt( GPSisr );
  NeoSerial1.begin( 9600 );
  Serial.begin(115200);
  /*while (!Serial)

    ;
    Serial.println("Serial, Yo.");*/
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
}

uint8_t data[] = BELLIGERENCE;
// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

static gps_fix         fix;

void loop() {
  toggleLEDs();
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
    }

    if (fix.valid.time)
    {
      bikeData.epochtime = fix.dateTime;
    }
    else
    {
      bikeData.epochtime = 999;
    }

    //Serial.println("Blorp:");
    //Serial.write((uint8_t * ) &aFix, sizeof(aFix));
    //Serial.println(fix_data.latitude());


    if (fix.valid.location) //only send data if it's valid
    {
      int retrycount;
      retrycount = 0;
      while (manager.sendtoWait((uint8_t * ) &bikeData, sizeof(bikeData), CHURCH_NODE) != RH_ROUTER_ERROR_NONE)
      {
        Serial.print(retrycount);
        if (retrycount > 10)
        {
          break;
        }
        retrycount++;
      }
      
      Serial.print(" time : ");
      Serial.print(bikeData.epochtime);
      Serial.print(" lat: ");
      Serial.print(bikeData.latitude);
      Serial.print(" lon: ");
      Serial.print(bikeData.longitude);
      Serial.println();
    }

  }
}
