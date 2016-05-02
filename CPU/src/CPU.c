/*
 ============================================================================
 Name        : CPU.c
 Author      : Andres Michel
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <mllibs/sockets/client.h>
#include <mllibs/sockets/package.h>
#include <commons/log.h>
#include <mllibs/log/logger.h>
#include <pthread.h>
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

	while(1){

	}
}

