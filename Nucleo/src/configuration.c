/*
 * configuration.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "configuration.h"

void configurar(){

	config = malloc(sizeof(Configuration));

	t_config* nConfig = config_create(NUCLEO_CONFIG_PATH);
	if(nConfig==NULL){
		//para debuggear desde eclipse
		nConfig = config_create(NUCLEO_CONFIG_PATH_ECLIPSE);
		if(nConfig==NULL){
			printf("No se encontro el archivo de configuracion.");
			exit (1);
		}
	}
	config->puerto_nucleo_cpu = config_get_int_value(nConfig,PUERTO_CPU);
	config->puerto_nucleo_prog = config_get_int_value(nConfig,PUERTO_PROG);
	config->ip_nucleo = config_get_string_value(nConfig,IP_NUCLEO);
	config->ip_umc = config_get_string_value(nConfig,IP_UMC);
	config->puerto_umc = config_get_int_value(nConfig,PUERTO_UMC);

	//planificador
	config->quantum = config_get_int_value(nConfig,QUANTUM);
	config->quantum_sleep = config_get_int_value(nConfig,QUANTUM_SLEEP);

	config->stack_size = config_get_int_value(nConfig,STACK_SIZE);

	//configuracion de log
	config->log_level = config_get_string_value(nConfig,LOG_LEVEL);
	config->log_file = config_get_string_value(nConfig,LOG_FILE);
	config->log_program_name = config_get_string_value(nConfig,LOG_PROGRAM_NAME);
	config->log_print_console = config_get_int_value(nConfig,LOG_PRINT_CONSOLE);

	//levanto los dispositivos de entrada/salida
	config->io_ids = config_get_array_value(nConfig,IO_IDS);
	int len=0;
	while(config->io_ids[len]!=NULL){
		len++;
	}
	config->io_length = len;
	char** io_sleep_aux = config_get_array_value(nConfig,IO_SLEEP);
	config->io_sleep = malloc(sizeof(int)*len);
	int i;
	for(i=0; i<len; i++){
		config->io_sleep[i] = atoi(io_sleep_aux[i]);
		free(io_sleep_aux[i]);
	}
	free(io_sleep_aux);
}

Configuration* getConfiguration(){
	return config;
}
