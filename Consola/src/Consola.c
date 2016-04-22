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
#include <commons/log.h>
#include "configuration.h"
#include "consola.h"


int main(int argc, char* argv[]) {

	//creo el log
	t_log* logger = log_create("consola.log","ELESTAC",true, LOG_LEVEL_DEBUG);

	Configuration* config = configurar();
	char* programa;
	FILE* fp;


	if(argc < 2){
		log_error(logger,"Consola debe recibir un programa ANSISOP como argumento\n");
		return EXIT_FAILURE;
	}

	log_info(logger,"Consola iniciada");

	programa = argv[argc - 1];

	log_info(logger,"Ejecutando: %s\n",programa);

	if((fp=fopen(programa,"r"))==NULL){
		log_error(logger,"Error al abrir el programa %s",programa);
		return EXIT_FAILURE;
	}

	while(!feof(fp)){
		printf("%c",fgetc(fp));
	}

	comunicacionConNucleo(config,logger);

	fclose(fp);
	return EXIT_SUCCESS;
}
