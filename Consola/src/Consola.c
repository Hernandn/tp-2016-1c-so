/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int main(void) {

	int my_socket, exit_consola = 1;
	struct sockaddr_in serv_addr;

	puts("Consola iniciada");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(8080);

	my_socket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connect(my_socket, (void*) &serv_addr, sizeof(serv_addr)) != 0) {
		perror("No se pudo conectar");
		return 1;
	}

	puts("Socket conectado");

	while (exit_consola) {
		char mensaje[1000];
		puts("Ingrese mensaje:");
		scanf("%s", mensaje);

		if(!strcmp(mensaje,"exit")){
			strcpy(mensaje,"Cerrando consola");
			exit_consola = 0;
		}
			send(my_socket, mensaje, strlen(mensaje), 0);
	}

	close(my_socket);
	return EXIT_SUCCESS;
}
