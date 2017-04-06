/**
 Cycle Schema
 
 Grabs bike gps data points from an sqlite db and draw lines
 **/

import de.bezier.data.sql.*;

/** CONFIG **/

// sqlite db path
String dbPath = "/Users/steve/Desktop/cs.sqlite";

int screenWidth = 1024;
int screenHeight = 768;

int lineWeight = 10;
float lineAlpha = 70; // 0-255
float screenPadding = 10;

float churchLat = -32.658;
float churchLon = 118.330;
float churchScale = 0.2;

int framerate = 6; // mainly for the gif animation
int updateFrequency = 6; // how often (in frames) to do a db update

/** PROPERTIES **/
SQLite db;
boolean dbConnected = false;
float x1, y1, x2, y2 = 0; // area bounds
PImage[] church = new PImage[20];
int churchFrame = 0;

Bike[] bikes;

int boundsUpdateTime = 0;

// setup
void setup() {

  frameRate(framerate);
  background(0);
  //fullScreen();
  size(1024, 768);
  noCursor();
  smooth();

  // bikes
  bikes  = new Bike[5];
  bikes[0] = new Bike(1, color(255, 0, 0));
  bikes[1] = new Bike(2, color(0, 255, 0));
  bikes[2] = new Bike(3, color(0, 0, 255));
  bikes[3] = new Bike(4, color(255, 0, 255));
  bikes[4] = new Bike(5, color(255, 255, 0));

  // load animated church images
  for (int i = 0; i < 20; i++) {
    String imageName = "church/church_sim_" + nf(i, 5) + ".png";
    church[i] = loadImage(imageName);
  }

  // setup db
  db = new SQLite(this, dbPath);

  if (db.connect()) {
    getBounds();
    for (int i=0; i<5; i++) {
      bikes[i].updateBike();
    }
    dbConnected = true;
  } else {
    println("eek. couldn't connect to db.");
  }
}

// draw
void draw() {

  // clear background
  background(0);

  // draw church
  churchFrame = (churchFrame+1) % 20;
  image(church[churchFrame], map(churchLat, x1, x2, screenPadding, screenWidth-screenPadding), map(churchLon, y1, y2, screenPadding, screenHeight-screenPadding), church[0].width * churchScale, church[0].height * churchScale);

  if (frameCount % updateFrequency == 0) {
    // update bikes
    if (dbConnected) {
      getBounds();
      for (int i=0; i<5; i++) {
        bikes[i].updateBike();
      }
    }
  }

  // draw bikes
  for (int i=0; i<5; i++) {
    bikes[i].drawBike();
  }
}

void getBounds() {

  db.query("SELECT * FROM bikes WHERE time > %d", boundsUpdateTime);

  while (db.next()) {

    BikeData b = new BikeData();
    db.setFromRow(b);
    //println(b);

    if (x1 == 0) {
      x1 = x2 = (float)b.lat;
      y1 = y2 = (float)b.lon;
    } else {
      x1 = (b.lat < x1) ? (float)b.lat : x1;
      x2 = (b.lat > x2) ? (float)b.lat : x2;
      y1 = (b.lon > y1) ? (float)b.lon : y1;
      y2 = (b.lon < y2) ? (float)b.lon : y2;
    }
  }

  println(x1, x2, y1, y2);
}

class Bike {

  public int bikeid;
  public int lastUpdateTime = 0;
  public color lineColour;
  public double pLat = 0.0;
  public double pLon = 0.0;

  public PGraphics bikeImage = createGraphics(screenWidth, screenHeight);

  public Bike(int bid, color lColour) {
    bikeid = bid;
    lineColour = lColour;
  }

  public void updateBike() {

    db.query("SELECT * FROM bikes WHERE bikeid = %d AND time > %d ORDER BY time", bikeid, lastUpdateTime);

    bikeImage.beginDraw();
    bikeImage.strokeWeight(lineWeight);
    bikeImage.stroke(lineColour, lineAlpha);

    while (db.next()) {

      BikeData b = new BikeData();
      db.setFromRow(b);
      //println(b);

      // first point 
      if (pLat == 0.0 && pLon == 0.0) {
        pLat = b.lat;
        pLon = b.lon;
      }

      // draw line, map bounds to screen 
      bikeImage.line(map((float)pLat, x1, x2, screenPadding, screenWidth-screenPadding), 
        map((float)pLon, y1, y2, screenPadding, screenHeight-screenPadding), 
        map((float)b.lat, x1, x2, screenPadding, screenWidth-screenPadding), 
        map((float)b.lon, y1, y2, screenPadding, screenHeight-screenPadding));

      // save for next time
      pLat = b.lat;
      pLon = b.lon;
      lastUpdateTime = b.time;
      if (lastUpdateTime > boundsUpdateTime) {
        boundsUpdateTime = lastUpdateTime;
      }
    }

    bikeImage.endDraw();
  }

  public void drawBike() {
    image(bikeImage, 0, 0);
  }
}

class BikeData {

  public int bikeid;
  public int time;
  public double lat;
  public double lon;
  public int batt;

  public String toString() {
    return String.format("%d\t%d\t%f\t%f\t%d", bikeid, time, lat, lon, batt);
  }
}