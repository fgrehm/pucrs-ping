build: pucrs-ping

pucrs-ping: main.c echo_request.*
	gcc -o pucrs-ping main.c echo_request.c
