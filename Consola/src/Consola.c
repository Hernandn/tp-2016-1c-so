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
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <mllibs/sockets/client.h>
#include <mllibs/sockets/package.h>
#include "configuration.h"
#include "consola.h"


int main(int argc, char* argv[]) {

	Configuration* config = configurar();
	char* programa;
	FILE* fp;


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

	comunicacionConNucleo(config);

	fclose(fp);
	return EXIT_SUCCESS;
}

Configuration* configurar(){

	Configuration* config = malloc(sizeof(Configuration));

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
