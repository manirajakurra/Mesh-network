import cx_Oracle
import re
import serial 
con = cx_Oracle.connect('apex/apex@xe')
cur = con.cursor()
ser = serial.Serial("/dev/ttyACM0",115200)
while 1:
	if re.match("(.*)(R|r)eceived_message(.*)",ser.readline()):
		message  = ser.readline()
                print (message)
                if re.match("(.*)(R|r)eceived_message_from(.*)",ser.readline()):
			Node_id = ser.readline()
                        print (Node_id)
       			cur.execute('INSERT INTO message_history VALUES (\''+str(message)+ '\',\''+str(Node_id)+ '\', sysdate)');
			del message
                	del Node_id
       		cur.execute('commit');
		print ("done")
cur.close()
con.close()
