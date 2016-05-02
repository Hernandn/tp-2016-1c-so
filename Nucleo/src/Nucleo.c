/*
 ============================================================================
 Name        : Nucleo.c
 Author      : Andres Michel
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/errno.h>
#include "configuration.h"
#include "Nucleo.h"
#include <mllibs/log/logger.h>

int main(void)
{
	Configuration* config = configurar();

	//creo el log
	initLogMutex(config->log_file, config->log_program_name, config->log_print_console, log_level_from_string(config->log_level));

	handleClients(config);

	return 0;
}

