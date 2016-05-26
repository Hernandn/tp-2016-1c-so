/*
 ============================================================================
 Name        : CPU.c
 Author      : Andres Michel
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "CPU.h"

int main(void){

	Configuration* config = configurar();

	//creo el log
	initLogMutex(config->log_file, config->log_program_name, config->log_print_console, log_level_from_string(config->log_level));

	arg_struct args;
	args.config=config;

	pthread_t hilo1;
	pthread_create(&hilo1,NULL,(void*)conectarConUMC,(void *)&args);
	pthread_t hilo2;
	pthread_create(&hilo2,NULL,(void*)conectarConNucleo,(void *)&args);

	pthread_join(hilo1,NULL);
	pthread_join(hilo2,NULL);

	return 0;
}

