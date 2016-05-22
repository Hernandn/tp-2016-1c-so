/*
 * configuration.h
 *
 *  Created on: 22/04/2016
 *      Author: hernan
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <commons/config.h>
#include <mllibs/log/logger.h>

#define UMC_CONFIG_PATH "../umc.conf"

#define PUERTO_SWAP "PUERTO_SWAP"
#define IP_SWAP "IP_SWAP"
#define PUERTO_UMC "PUERTO_UMC"
#define IP_UMC "IP_UMC"
#define CANTIDAD_PAGINAS "CANTIDAD_PAGINAS"
#define TAMANIO_PAGINA "TAMANIO_PAGINA"

typedef struct Configuration {

	int puerto_swap;
	char* ip_swap;
	int puerto_umc;
	char* ip_umc;
	int cantidad_paginas;
	int size_pagina;
	char* log_level;
	char* log_file;
	char* log_program_name;
	int log_print_console;

} Configuration;

Configuration* config;

void configurar();

#endif /* CONFIGURATION_H_ */
