all: envio_socket recepcao_socket

envio_socket: envio_socket.c
	gcc -o envio_socket envio_socket.c

recepcao_socket: recepcao_socket.c
	gcc -o recepcao_socket recepcao_socket.c
