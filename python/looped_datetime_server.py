import socket
import time
from datetime import datetime



HOST = str(input("What is your server IP?: "))                 
PORT = int(input("What is your initial port?: "))




def connect_and_send( data, HOST, PORT):
    my_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    my_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    my_socket.bind((HOST, PORT))
    my_socket.listen(1)

    print('socket binded')
    print('socket is listening')

    conn, addr = my_socket.accept()
    print ('Connected by {}'.format(addr))

    conn.sendall(data)

    conn.close()
    my_socket.close()




while True:
    today = datetime.now()
    my_date = today.strftime("%d/%m/%Y %H:%M:%S")
    my_date = str.encode('{}{}'.format(my_date, '\n'))

 
    NEW_PORT = int(input("Type port number to change: "))
    NEW_PORT_bytes = b'%d\n' % NEW_PORT

    connect_and_send(NEW_PORT_bytes, HOST, PORT)
    print ('Change port request sent...')
    
    connect_and_send(my_date, HOST, NEW_PORT)
    print ('Daytime sent')




