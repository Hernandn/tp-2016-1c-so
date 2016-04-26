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

int main(void)
{
	//creo el log
	t_log* logger = log_create("nucleo.log","ELESTAC",true, LOG_LEVEL_DEBUG);
	Configuration* config = configurar(logger);

	handleClients(config,logger);

	return 0;
}

