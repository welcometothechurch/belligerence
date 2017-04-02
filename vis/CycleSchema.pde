/**
 Cycle Schema
 
 Grabs bike gps data points from an sqlite db and draw lines
 **/

import de.bezier.data.sql.*;

/** CONFIG **/

// sqlite db path
String dbPath = "/Users/steve/Desktop/cs.sqlite";

// line colours
color bCol1 = color(255, 0, 0);
color bCol2 = color(0, 255, 0);
color bCol3 = color(0, 0, 255);
color bCol4 = color(255, 0, 255);
color bCol5 = color(255, 255, 0);

int lineWeight = 10;
float lineAlpha = 70; // 0-255
float screenPadding = 20;

/** PROPERTIES **/
SQLite db;
boolean dbConnected = false;
float x1, y1, x2, y2 = 0; // area bounds

// setup
void setup() {

  frameRate(1/60); // 1 frame / min
  background(0);
  fullScreen();
  noCursor();
  smooth();

  db = new SQLite(this, dbPath);

  if (db.connect()) {
    drawBikes();
    dbConnected = true;
  }
}

// draw
void draw() {

  // clear background
  background(0);

  // draw bike lines
  if (dbConnected) {
    drawBikes();
  }
}

void drawBikes() {
  getBounds();
  // change the order if you like
  drawBike(1, bCol1);
  drawBike(2, bCol2);
  drawBike(3, bCol3);
  drawBike(4, bCol4);
  drawBike(5, bCol5);
}

void drawBike(int bikeid, color col) {

  db.query("SELECT * FROM bikes WHERE bikeid = %d ORDER BY time", bikeid);

  stroke(col, lineAlpha);
  strokeWeight(lineWeight);

  // previous points
  double pLat = 0.0;
  double pLon = 0.0;

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
    line(map((float)pLat, x1, x2, screenPadding, width-screenPadding), 
      map((float)pLon, y1, y2, screenPadding, height-screenPadding), 
      map((float)b.lat, x1, x2, screenPadding, width-screenPadding), 
      map((float)b.lon, y1, y2, screenPadding, height-screenPadding));

    // save for next time
    pLat = b.lat;
    pLon = b.lon;
  }
}

void getBounds() {

  x1 = x2 = y1 = y2 = 0;

  db.query("SELECT * FROM bikes");

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
  
  println("%f, %f, %,f %f", x1, x2, y1, y2);
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