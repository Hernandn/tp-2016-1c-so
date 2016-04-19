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

int main(void)
{
	//probando levantar archivo de configuracion
	t_config* nConfig = config_create(NUCLEO_CONFIG_PATH);
	printf("Archivo de configuracion: \n");
	printf("%s: %d\n",PUERTO_PROG,config_get_int_value(nConfig,PUERTO_PROG));
	printf("%s: %d\n",PUERTO_CPU,config_get_int_value(nConfig,PUERTO_CPU));
	printf("%s: %d\n",QUANTUM,config_get_int_value(nConfig,QUANTUM));
	printf("%s: %d\n",QUANTUM_SLEEP,config_get_int_value(nConfig,QUANTUM_SLEEP));

	//hay que ver como sacar el length de este array de strings
	char** array1 = config_get_array_value(nConfig,SEM_IDS);
	int i;
	for(i=0;i<3;i++){
		printf("array[%d]: %s\n",i,array1[i]);
	}
	/*
	char** array2 = config_get_array_value(nConfig,SEM_INIT);
	char** array3 = config_get_array_value(nConfig,IO_IDS);
	char** array4 = config_get_array_value(nConfig,IO_SLEEP);
	char** array5 = config_get_array_value(nConfig,SHARED_VARS);
	*/




	//--------------

	/******sockets*******/
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(8080);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0)
	{
		perror("fallo el bind capo!!\n");
		return 1;
	}
	printf("Estoy escuchando\n");
	listen(servidor, 100);

	struct sockaddr_in direccionCliente;
	unsigned int tamanoDireccion = sizeof(struct sockaddr_in);
	int cliente = accept(servidor, (void*)  &direccionCliente, &tamanoDireccion);

	printf("Recibi una conexion en %d!!\n", cliente);
	send(cliente, "Hola a todos y todas\n", 20, 0 );

	char *buffer = malloc(15);

	int bytesRecibidos = recv(cliente, buffer, 14, 0);
	if(bytesRecibidos < 0)
	{
		perror("O te desconectaste o algo paso\n");
		return 1;
	}

		buffer[bytesRecibidos]= '\0';
		printf("Me llegaron %d bytes con %s\n", bytesRecibidos, buffer);

		free(buffer);

	return 0;
}

