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

	Configuration* config = NULL;
	char* programa = NULL;
	FILE* fp;
	int i;


	if(argc < 2){
		log_error(logger,"Consola debe recibir un programa ANSISOP como argumento\n");
		mostrar_ayuda();
		return EXIT_FAILURE;
	}

	/*
	 *
	 * Uso: Consola [-f] programa [-c] "archivo de configuracion"
	 *
	 * Opciones:
	 *  -h: Muestra ayuda.
	 *
	 * Archivos:
	 *  -f: Programa ansisop.
	 *  -c: Archivo de configuracion.
	 *
	 */

	//Leo los argumentos, evito el 0 porque es el path de ejecucion
	for(i=1;i<argc;i++){
		if(*(argv[i])=='-' && strlen(argv[i]) == 2){
			//Si el argumento empieza con "-" y su tamaño es 2 tiene que ser una opcion
			switch(*(argv[i]+1)){
				case 'c':
					config = configurar(argv[++i]);
					break;
				case 'f':
					programa = (argv[++i]);
					break;
				case 'h':
					mostrar_ayuda();
					exit (EXIT_SUCCESS);
				default:
					//Si no es ninguno de los ateriores el argumento es invalido
					printf("Mostraria argumentos invalidos");
					//invalid_argument(*(argv[i]+1));
			}
		} else {
			//En caso contrario se asume que es el path del programa o del archivo de configuracion en ese orden.
			if(programa == NULL) programa = argv[i];
			else if(config == NULL) config = configurar(*argv[i]);
		}
	}

	//Si no esta espesificado el archivo de configuracion se toma el default.
	if(config == NULL){
		config = configurar(getenv(CONSOLA_CONFIG_PATH));
	}

	log_info(logger,"Consola iniciada");

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
