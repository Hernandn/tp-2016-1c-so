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

	initLogMutex(DEFAULT_LOG_FILE,"ELESTAC",true,LOG_LEVEL_DEBUG);

	//Cargo la configuracion
	parametros = interpretar_parametros(argc, argv);

	//Si hay un archivo log por configuracion uso el nuevo
	if(strlen(parametros->config->log_file) != 0 && !strcmp(parametros->config->log_file,DEFAULT_LOG_FILE)){
		logDestroy();
		initLogMutex(parametros->config->log_file,"ELESTAC",true,LOG_LEVEL_DEBUG);
	}

	logInfo("Consola iniciada");

	if(parametros->programa == NULL){
		logDebug("No fue espesificado un programa, en este momento la consola deberia permitir usar linea de comandos para acceder al archivo");
		return EXIT_FAILURE;
	}

	logInfo("Ejecutando: %s\n",parametros->programa);

	comunicacionConNucleo(parametros->config, parametros->programa);

	logInfo("Fin programa\n");

	//Libero la memoria
	liberar_parametros(parametros);

	//Cierro el logger
	logDestroy();

	return EXIT_SUCCESS;
}
