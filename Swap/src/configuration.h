/*
 * configuration.h
 *
 *  Created on: 22/04/2016
 *      Author: hernan
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define SWAP_CONFIG_PATH "../swap.conf"
#define SWAP_CONFIG_PATH_ECLIPSE "swap.conf"

#define PUERTO_SWAP "PUERTO_SWAP"
#define IP_SWAP "IP_SWAP"


typedef struct Configuration {
	int puerto_swap;
	char* ip_swap;
	char* log_level;
	char* log_file;
	char* log_program_name;
	int log_print_console;
} Configuration;

#endif /* CONFIGURATION_H_ */
