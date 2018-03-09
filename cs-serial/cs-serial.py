#format: bikeid,time,lat,long,batt
#3,538585518,-320642233,1158780068,407

import sqlite3
import serial
import os
import sys
import signal
import curses

class errorTally:
	nullLine = 0
	length = 0
	format = 0
	bikeID = 0
	time = 0
	lat = 0
	long = 0
	volts = 0
	sql = 0
	sptl = 0
	port = 0
errors = errorTally()

class bike:
	seen = ''
	lat = ''
	long = ''
	bat = ''
bikes = [bike() for i in range(5)]

stdscr = curses.initscr()
curses.noecho()
curses.curs_set(0)
errorWin = curses.newwin(14,22,2,58)
bikesWin = curses.newwin(8,55,2,0)

SQLdb = '/home/pi/cs/cs.sqlite'

# Raw data stored as CSV:
outputFile = './data/cs_data.csv'

def sigint_handler(signal, frame):
	conn.commit()
	conn.close()
	logFile.close()
	curses.nocbreak()
	curses.echo()
	curses.endwin()
	print("Exiting")
	sys.exit(0)
signal.signal(signal.SIGINT, sigint_handler)

connSQL = sqlite3.connect(SQLdb)
c = connSQL.cursor()

logFile = open(outputFile,'a')

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=10)

errorWin.addstr(0,0,' Error Tally')
errorWin.addstr(1,0,'--------------')
errorWin.addstr(2,0,'    Null Line:')
errorWin.addstr(3,0,'Packet Length:')
errorWin.addstr(4,0,'       Format:')
errorWin.addstr(5,0,'      Bike ID:')
errorWin.addstr(6,0,'    Timestamp:')
errorWin.addstr(7,0,'     Latitude:')
errorWin.addstr(8,0,'    Longitude:')
errorWin.addstr(9,0,'        Volts:')
errorWin.addstr(10,0,'      SQLite3:')
errorWin.addstr(11,0,'  Serial Port:')
errorWin.refresh()

#                      3    538585518   -32.0642233   115.8780068    407
timeNow = ''
bikesWin.addstr(0,0,'Bike | Last Seen | Latitude    | Longitude   | Battery')
bikesWin.addstr(1,0,'------------------------------------------------------')


while True:
	try:
		line = ser.readline()
	except:
		errors.port += 1
		errorWin.addstr(11,16,str(errors.port))
		errorWin.refresh()
	else:
		if line != '':
			if len(line) == 39: # Check line length is valid
				line = line.decode('utf-8') # Convert bytes to string
				if line.count(',') == 4: # Check how many commas are in it
					logFile.write(line)
					line = line[:-2] # Strip the \r\n from the end
					line = line.split(',') # Now a list of strings
					# Check the field lengths are valid
					if len(line[0]) == 1: # Bike ID
						if len(line[1]) == 9: # Time
							if len(line[2]) == 10: # Lattitude
								if len(line[3]) == 10: # Longitude
									if len(line[4]) == 3: # Battery Volts
										# add decimal points to lat/long strings:
										line[2] = line[2][0:3] + "." + line[2][3:]
										line[3] = line[3][0:3] + "." + line[3][3:]
										sqlCmd = 'INSERT INTO bikes VALUES (' + line[0] + ',' + line[1] + ',' + line[2] + ',' + line[3] + ',' + line[4] + ')'
										bikes[int(line[0])-1].seen = line[1]
										bikes[int(line[0])-1].lat = line[2]
										bikes[int(line[0])-1].long = line[3]
										bikes[int(line[0])-1].bat = line[4]
										timeNow = line[1]
										bikesWin.addstr(int(line[0])+1,0,'  %s  | %s | %s | %s |  %s  ' % (line[0], line[1], line[2], line[3], line[4]))
										bikesWin.addstr(7,0,'  Now: %s' % line[1])
										bikesWin.refresh()
										try:
											c.execute(sqlCmd)
											connSQL.commit()
											line = ''
										except:
											errors.sql += 1
											errorWin.addstr(10,16,str(errors.sql))
											errorWin.refresh()
									else:
										errors.volts += 1
										errorWin.addstr(9,16,str(errors.volts))
										errorWin.refresh()
								else:
									errors.long += 1
									errorWin.addstr(8,16,str(errors.long))
									errorWin.refresh()
							else:
								errors.lat += 1
								errorWin.addstr(7,16,str(errors.lat))
								errorWin.refresh()
						else:
							errors.time += 1
							errorWin.addstr(6,16,str(errors.time))
							errorWin.refresh()
					else:
						errors.bikeID += 1
						errorWin.addstr(5,16,str(errors.bikeID))
						errorWin.refresh()
				else:
					errors.format += 1
					errorWin.addstr(4,16,str(errors.format))
					errorWin.refresh()
			else:
				errors.length += 1
				errorWin.addstr(3,16,str(errors.length))
				errorWin.refresh()
		else:
			errors.nullLine += 1
			errorWin.addstr(2,16,str(errors.nullLine))
			errorWin.refresh()
