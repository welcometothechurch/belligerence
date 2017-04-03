Python script for reading CycleSchema data from a serial port and loading it into an SQLite database.

The script expects to find a SQLite db file called "cs.sqlite", rename one of the supplied samples as needed.

Script assumes serial data is coming in on port ttyACM0 before it starts, and will likely fail to recover from having its precious data flow interrupted. Most others faults in the stream should be ignored successfully.

TODO: Detect serial ports on startup and choose appropriately.

TODO: Increase serial port readline() timeout and deal gracefully with unplug-replug of church arduino node.

TODO: Packet and error counters, replace scrolling wall of text with sensible tally table.

TODO: Keep track of when we last heard from each bike, display in a table.
