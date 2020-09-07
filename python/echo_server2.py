import socket
from datetime import date

HOST = '192.168.8.106'                 # Symbolic name meaning all available interfaces
PORT = 10              # Arbitrary non-privileged port


today = date.today()
d1 = today.strftime("%d/%m/%Y")
d1 = str.encode('{}{}'.format(d1, '\n'))


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

s.bind((HOST, PORT))
s.listen(1)


conn, addr = s.accept()
print ('Connected by {}'.format(addr))
print ('Port change completed!!!')
conn.sendall(d1)
print ('Daytime sent')

conn.close()



