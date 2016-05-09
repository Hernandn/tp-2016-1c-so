/*
 ============================================================================
 Name        : UMC.c
 Author      : Andres Michel
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "UMC.h"
/*
 * Programa principal.
 * Crea un socket servidor y se mete en un select() a la espera de clientes.
 * Cuando un cliente se conecta, le atiende y lo añade al select() y vuelta
 * a empezar.
 */
int main(void) {

	Configuration* config = configurar();

	//creo el log
	initLogMutex(config->log_file, config->log_program_name, config->log_print_console, log_level_from_string(config->log_level));

	inicializarUMC(config);

	handleClients(config);

	logDestroy();

	return EXIT_SUCCESS;
}


