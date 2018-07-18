import cx_Oracle
import re
import serial 
con = cx_Oracle.connect('apex/apex@xe')
cur = con.cursor()
ser = serial.Serial("/dev/ttyACM0",115200)
while 1:
	if re.match("(.*)(R|r)eceived_message(.*)",ser.readline()):
       		cur.execute('INSERT INTO message_history VALUES (\''+str(ser.readline())+ '\')');
       		cur.execute('commit');
cur.close()
con.close()
