/*
 ============================================================================
 Name        : Nucleo.c
 Author      : Andres Michel
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/errno.h>
#include "configuration.h"
#include "Nucleo.h"

int main(void)
{
	//probando levantar archivo de configuracion
	/*
	t_config* nConfig = config_create(NUCLEO_CONFIG_PATH);
	printf("Archivo de configuracion: \n");
	printf("%s: %d\n",PUERTO_PROG,config_get_int_value(nConfig,PUERTO_PROG));
	printf("%s: %d\n",PUERTO_CPU,config_get_int_value(nConfig,PUERTO_CPU));
	printf("%s: %d\n",QUANTUM,config_get_int_value(nConfig,QUANTUM));
	printf("%s: %d\n",QUANTUM_SLEEP,config_get_int_value(nConfig,QUANTUM_SLEEP));

	//hay que ver como sacar el length de este array de strings

	char** array1 = config_get_array_value(nConfig,SEM_IDS);
	int i;
	for(i=0;array1[i]!=NULL;i++){
		printf("array[%d]: %s\n",i,array1[i]);
	}*/
	/*
	char** array2 = config_get_array_value(nConfig,SEM_INIT);
	char** array3 = config_get_array_value(nConfig,IO_IDS);
	char** array4 = config_get_array_value(nConfig,IO_SLEEP);
	char** array5 = config_get_array_value(nConfig,SHARED_VARS);
	*/


	handleClients();

	return 0;
}

