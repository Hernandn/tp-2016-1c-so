/*
 * configuration.h
 *
 *  Created on: 22/04/2016
 *      Author: hernan
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define CPU_CONFIG_PATH "../cpu.conf"

#define PUERTO_NUCLEO "PUERTO_NUCLEO"
#define IP_NUCLEO "IP_NUCLEO"
#define PUERTO_UMC "PUERTO_UMC"
#define IP_UMC "IP_UMC"


typedef struct Configuration {
	int puerto_nucleo;
	char* ip_nucleo;
	int puerto_umc;
	char* ip_umc;
} Configuration;

#endif /* CONFIGURATION_H_ */
