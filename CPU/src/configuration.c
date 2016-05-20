/*
 * configuration.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "configuration.h"

Configuration* configurar(){

	Configuration* config = malloc(sizeof(Configuration));

	t_config* nConfig = config_create(CPU_CONFIG_PATH);
	if(nConfig==NULL){
		//para debuggear desde eclipse
		nConfig = config_create(CPU_CONFIG_PATH_ECLIPSE);
		if(nConfig==NULL){
			printf("No se encontro el archivo de configuracion.");
			exit (1);
		}
	}
	config->puerto_nucleo=config_get_int_value(nConfig,PUERTO_NUCLEO);
	config->ip_nucleo = config_get_string_value(nConfig,IP_NUCLEO);
	config->puerto_umc=config_get_int_value(nConfig,PUERTO_UMC);
	config->ip_umc = config_get_string_value(nConfig,IP_UMC);
	//configuracion de log
	config->log_level = config_get_string_value(nConfig,LOG_LEVEL);
	config->log_file = config_get_string_value(nConfig,LOG_FILE);
	config->log_program_name = config_get_string_value(nConfig,LOG_PROGRAM_NAME);
	config->log_print_console = config_get_int_value(nConfig,LOG_PRINT_CONSOLE);

	return config;
}

