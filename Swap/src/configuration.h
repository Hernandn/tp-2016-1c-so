/*
 * configuration.h
 *
 *  Created on: 22/04/2016
 *      Author: hernan
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define SWAP_CONFIG_PATH "../swap.conf"

#define PUERTO_SWAP "PUERTO_SWAP"
#define IP_SWAP "IP_SWAP"


typedef struct Configuration {
	int puerto_swap;
	char* ip_swap;
} Configuration;

#endif /* CONFIGURATION_H_ */
