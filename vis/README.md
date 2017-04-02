CYCLESCHEMA VIS
=======

Built using Processing 3.3 (processing.org)

Grabs bikedata from SQLite db and draws a coloured line for each bike

Change line colours, weight, opacity, padding etc at the top of the sketch.

Add the SQLite library to your Processing installation.
Sketch > Import Library > Add Library...
Search for BezierSQLib

SQLite DB
CREATE TABLE "bikes" ( `bikeid` INTEGER, `time` INTEGER, `lat` REAL, `lon` REAL, `batt` INTEGER )
