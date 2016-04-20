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
#include <commons/log.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include "configuration.h"

struct configuration {
	int puerto_nucleo;
	char* ip_nucleo;
};

struct configuration configurar ();

int main(int argc, char* argv[]) {

	struct configuration config = configurar();
	char* programa;

	if(argc < 2){
		puts("Consola debe recibir un programa ANSISOP como argumento\n");
		return EXIT_FAILURE;
	}

	puts("Consola iniciada");

	programa = argv[argc - 1];

	printf("Ejecutando: %s\n",programa);
/*
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
		char* buffer = malloc(5);

		int bytesRecibidos = recv(my_socket, buffer, 4, MSG_WAITALL);
		if (bytesRecibidos < 0) {
			perror("El chabón se desconectó o bla.");
			return 1;
		}

		buffer[bytesRecibidos] = '\0';
		printf("Me llegaron %d bytes con %s", bytesRecibidos, buffer);

		free(buffer);

		puts("");
		puts("Ingrese mensaje:");
		scanf("%s", mensaje);

		if(!strcmp(mensaje,"exit")){
			strcpy(mensaje,"Cerrando consola");
			exit_consola = 0;
		}
			send(my_socket, mensaje, strlen(mensaje), 0);
	}

	close(my_socket);*/

	return EXIT_SUCCESS;
}

struct configuration configurar(){

	struct configuration config;

	t_config* nConfig = config_create(CONSOLA_CONFIG_PATH);
	if(nConfig==NULL){
		puts("No se encontro el archivo de configuracion");
		exit (1);
	}
	config.puerto_nucleo=config_get_int_value(nConfig,PUERTO_NUCLEO);
	printf("Puerto de nucleo: %d\n",config.puerto_nucleo);
	config.ip_nucleo = config_get_string_value(nConfig,IP_NUCLEO);
	printf("Ip de nucleo: %s\n",config.ip_nucleo);

	return config;
}
