/*
 ============================================================================
 Name        : Nucleo.c
 Author      : Andres Michel
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/errno.h>

int main(void)
{
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(8080);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0)
	{
		perror("fallo el bind capo!!");
		return 1;
	}
	printf("Estoy escuchando\n");
	listen(servidor, 100);

	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion = sizeof(struct sockaddr_in);
	int cliente = accept(servidor, (void*)  &direccionCliente, &tamanoDireccion);

	printf("Recibi una conexion en %d!!\n", cliente);
	send(cliente, "Hola a todos y todas\n", 50, 0 );

	char *buffer = malloc(15);

	int bytesRecibidos = recv(cliente, buffer, 14, 0);
	if(bytesRecibidos < 0)
	{
		perror("O te desconectaste o algo paso\n");
		return 1;
	}

		buffer[bytesRecibidos]= '\0';
		printf("Me llegaron %d bytes con %s", bytesRecibidos, buffer);

		free(buffer);

	return 0;
}

