/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "consola.h"

int main(int argc, char* argv[]){

	Parameters* parametros;
	FILE* fp;

	//Crear el logger
	t_log* logger = log_create("consola.log","ELESTAC",true, LOG_LEVEL_DEBUG);

	//Obtener parametros
	parametros = interpretar_parametros(argc, argv, logger);

	log_info(logger,"Consola iniciada");

	log_info(logger,"Ejecutando: %s\n",parametros->programa);

	//Esto lo voy a poner en un .c aparte que cree algun tipo de estructura serializada para despues pasarle al comunicacion con nucleo
	if((fp=fopen(parametros->programa,"r"))==NULL){
		log_error(logger,"Error al abrir el programa %s",parametros->programa);
		return EXIT_FAILURE;
	}

	while(!feof(fp)){
		printf("%c",fgetc(fp));
	}
	//----------------------------------------------------------------------------------------------------------------------------/

	comunicacionConNucleo(parametros->config,logger);

	log_info(logger,"Fin programa\n");

	//Libero la memoria
	liberar_parametros(parametros);

	//Cierro el logger
	log_destroy(logger);

	//Cierro el archivo ansisop
	fclose(fp);

	return EXIT_SUCCESS;
}
