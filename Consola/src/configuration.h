/*
 * configuration.h
 *
 *  Created on: 19/4/2016
 *      Author: utnso
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define CONSOLA_CONFIG_PATH "CONF_CONSOLA"
#define IP_NUCLEO "IP_NUCLEO"
#define PUERTO_NUCLEO "PUERTO_NUCLEO"

typedef struct Configuration {
	int puerto_nucleo;
	char* ip_nucleo;
} Configuration;

#endif /* CONFIGURATION_H_ */
