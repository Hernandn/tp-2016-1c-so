/*
 * configuration.c
 *
 *  Created on: 8/5/2016
 *      Author: utnso
 */

#include "configuration.h"

void configurar(){

	config = malloc(sizeof(Configuration));

	t_config* nConfig = config_create(UMC_CONFIG_PATH);
	if(nConfig==NULL){
		printf("No se encontro el archivo de configuracion.");
		exit (1);
	}
	config->puerto_swap=config_get_int_value(nConfig,PUERTO_SWAP);
	config->ip_swap = config_get_string_value(nConfig,IP_SWAP);
	config->puerto_umc=config_get_int_value(nConfig,PUERTO_UMC);
	config->ip_umc = config_get_string_value(nConfig,IP_UMC);
	config->cantidad_paginas = config_get_int_value(nConfig,CANTIDAD_PAGINAS);
	config->size_pagina = config_get_int_value(nConfig,TAMANIO_PAGINA);
	//configuracion de log
	config->log_level = config_get_string_value(nConfig,LOG_LEVEL);
	config->log_file = config_get_string_value(nConfig,LOG_FILE);
	config->log_program_name = config_get_string_value(nConfig,LOG_PROGRAM_NAME);
	config->log_print_console = config_get_int_value(nConfig,LOG_PRINT_CONSOLE);

}
