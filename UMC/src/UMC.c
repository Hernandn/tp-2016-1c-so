/*
 ============================================================================
 Name        : UMC.c
 Author      : Andres Michel
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include "UMC.h"
#include "configuration.h"
#include <mllibs/sockets/server.h>
#include <mllibs/sockets/package.h>
#include <commons/config.h>
#include <unistd.h>
#include <commons/log.h>

#define MAX_CLIENTES 10


/*
 * Programa principal.
 * Crea un socket servidor y se mete en un select() a la espera de clientes.
 * Cuando un cliente se conecta, le atiende y lo añade al select() y vuelta
 * a empezar.
 */
int main(void) {

	t_log* logger = log_create("cpu.log","ELESTAC",true, LOG_LEVEL_DEBUG);
	Configuration* config = configurar(logger);
	handleClients(config,logger);
	return EXIT_SUCCESS;
}


