#format: bikeid,time,lat,long,batt
#3,538585518,-320642233,1158780068,407

# SQLite DB File:
dbFile = '/home/pi/cs/cs.sqlite'

# Raw data stored as CSV:
outputFile = '/home/pi/cs/cs_data.csv'

import sqlite3
import serial
import os
import sys
import signal

def sigint_handler(signal, frame):
	conn.commit()
	conn.close()
	logFile.close()
	print("Exiting")
	sys.exit(0)
signal.signal(signal.SIGINT, sigint_handler)

conn = sqlite3.connect(dbFile)
c = conn.cursor()

logFile = open(outputFile,'a')

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=10)

while True:
	try:
		line = ser.readline()
	except:
		print('Serial Port Error')
	if line != '':
		if len(line) == 39: # Check line length is valid
			line = line.decode('utf-8') # Convert bytes to string
			if line.count(',') == 4: # Check how many commas are in it
				logFile.write(line)
				line = line[:-2] # Strip the \r\n from the end
				#print(line)
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
									print(line)
									sqlCmd = 'INSERT INTO bikes VALUES (' + line[0] + ',' + line[1] + ',' + line[2] + ',' + line[3] + ',' + line[4] + ')'
									#print(sqlCmd)
									try:
										c.execute(sqlCmd)
										conn.commit()
										line = ''
									except:
										print('SQL Command Error')
								else:
									print('Volts Error')
							else:
								print('Longitude Error')
						else:
							print('Lattitude Error')
					else:
						print('Time Error')
				else:
					print('Bike ID Error')
			else:
				print('Format Error')
		else:
			print('Packet Length Error')
	else:
		print('Null Line Error')
