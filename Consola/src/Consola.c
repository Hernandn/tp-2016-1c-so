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
#include <commons/config.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <mllibs/sockets/client.h>
#include <mllibs/sockets/package.h>
#include "configuration.h"
#include "consola.h"

struct configuration {
	int puerto_nucleo;
	char* ip_nucleo;
};

struct configuration* configurar ();

int main(int argc, char* argv[]) {

	struct configuration* config = configurar();
	char* programa;
	FILE* fp;
	int socket;
	int buffer;
	int error;

	if(argc < 2){
		puts("Consola debe recibir un programa ANSISOP como argumento\n");
		return EXIT_FAILURE;
	}

	puts("Consola iniciada");

	programa = argv[argc - 1];

	printf("Ejecutando: %s\n",programa);

	if((fp=fopen(programa,"r"))==NULL){
		printf("Error al abrir el programa %s",programa);
		return EXIT_FAILURE;
	}

	while(!feof(fp)){
		printf("%c",fgetc(fp));
	}

	socket = abrirConexionInetConServer(config->ip_nucleo,config->puerto_nucleo);
	error = leerSocketClient(socket, (char *)&buffer, sizeof(int));

	if (error < 1)
	{
			printf ("Me han cerrado la conexión\n");
			exit(-1);
	}

	printf ("Soy la consola %d\n", buffer);

	Package package;
	while (1)
	{
		fillPackage(&package,ANSISOP_PROGRAM,"20,200,64");
		//escribirSocket(socket, (char *)&buffer, sizeof(int));
		char* serializedPkg = serializarMensaje(&package);
		escribirSocketClient(socket, (char *)serializedPkg, getLongitudPackage(&package));

		sleep(3);
	}

	close(socket);
	fclose(fp);
	return EXIT_SUCCESS;
}

struct configuration* configurar(){

	struct configuration* config = malloc(sizeof(struct configuration));

	t_config* nConfig = config_create(CONSOLA_CONFIG_PATH);
	if(nConfig==NULL){
		puts("No se encontro el archivo de configuracion");
		exit (1);
	}
	config->puerto_nucleo=config_get_int_value(nConfig,PUERTO_NUCLEO);
	printf("Puerto de nucleo: %d\n",config->puerto_nucleo);
	config->ip_nucleo = config_get_string_value(nConfig,IP_NUCLEO);
	printf("Ip de nucleo: %s\n",config->ip_nucleo);

	return config;
}
