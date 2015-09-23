all: envio_socket recepcao_socket

envio_socket: envio_socket.c echo_request.*
	gcc -o envio_socket envio_socket.c echo_request.c

recepcao_socket: recepcao_socket.c
	gcc -o recepcao_socket recepcao_socket.c
