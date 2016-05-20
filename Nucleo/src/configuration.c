/*
 * configuration.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "configuration.h"

Configuration* configurar(){

	Configuration* config = malloc(sizeof(Configuration));

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

	//configuracion de log
	config->log_level = config_get_string_value(nConfig,LOG_LEVEL);
	config->log_file = config_get_string_value(nConfig,LOG_FILE);
	config->log_program_name = config_get_string_value(nConfig,LOG_PROGRAM_NAME);
	config->log_print_console = config_get_int_value(nConfig,LOG_PRINT_CONSOLE);

	return config;
}
