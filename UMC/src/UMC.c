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

	pthread_t hilo1;

	//Leo la configuracion del archivo y lo guardo en una variable global (config)
	configurar();

	//creo el log
	initLogMutex(config->log_file, config->log_program_name, config->log_print_console, log_level_from_string(config->log_level));

	//Mutex para manejar el retardo de config
	pthread_mutex_init(&retardo_mutex,NULL);

	inicializarUMC(config);

	pthread_mutex_lock(&retardo_mutex);
	pthread_create(&hilo1,NULL,(void*)handleComandos,NULL);
	pthread_mutex_unlock(&retardo_mutex);
	handleClients();
    pthread_cancel(hilo1);

	logDestroy();

	return EXIT_SUCCESS;
}


