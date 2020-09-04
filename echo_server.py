# Echo server program
import socket

HOST = ''                 # Symbolic name meaning all available interfaces
PORT = 27              # Arbitrary non-privileged port

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

s.bind((HOST, PORT))
s.listen(1)

conn, addr = s.accept()
print ('Connected by {}'.format(addr))

NEW_PORT = int(input())
NEW_PORT_bytes = b'%d\n' % NEW_PORT

conn.sendall(NEW_PORT_bytes)
print ('Change port request sent...')
conn.close()


