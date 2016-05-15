/*
 * configuration.h
 *
 *  Created on: 22/04/2016
 *      Author: hernan
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <commons/config.h>

#define CPU_CONFIG_PATH "../cpu.conf"
#define CPU_CONFIG_PATH_ECLIPSE "cpu.conf"


#define PUERTO_NUCLEO "PUERTO_NUCLEO"
#define IP_NUCLEO "IP_NUCLEO"
#define PUERTO_UMC "PUERTO_UMC"
#define IP_UMC "IP_UMC"


typedef struct Configuration {
	int puerto_nucleo;
	char* ip_nucleo;
	int puerto_umc;
	char* ip_umc;
	char* log_level;
	char* log_file;
	char* log_program_name;
	int log_print_console;
} Configuration;

#endif /* CONFIGURATION_H_ */
