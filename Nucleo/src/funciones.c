/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <commons/config.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <mllibs/sockets/server.h>
#include <mllibs/sockets/client.h>
#include <mllibs/sockets/package.h>
#include "Nucleo.h"
#include "configuration.h"

Configuration* configurar(t_log* logger){

	Configuration* config = malloc(sizeof(Configuration));

	t_config* nConfig = config_create(NUCLEO_CONFIG_PATH);
	if(nConfig==NULL){
		//para debuggear desde eclipse
		nConfig = config_create(NUCLEO_CONFIG_PATH_ECLIPSE);
		if(nConfig==NULL){
			log_error(logger,"No se encontro el archivo de configuracion.");
			exit (1);
		}
	}
	config->puerto_nucleo_cpu=config_get_int_value(nConfig,PUERTO_CPU);
	config->puerto_nucleo_prog=config_get_int_value(nConfig,PUERTO_PROG);
	config->ip_nucleo = config_get_string_value(nConfig,IP_NUCLEO);

	return config;
}


void handleClients(Configuration* config, t_log* logger){

	arg_struct args;
	args.logger = logger;

	inicializarArraySockets(&args);

	//abrir server para escuchar CPUs
	args.socketServerCPU = abrirSocketInetServer(config->ip_nucleo,config->puerto_nucleo_cpu);
	if (args.socketServerCPU == -1)
	{
		perror ("Error al abrir servidor para CPUs");
		exit (-1);
	}
	//abrir server para escuchar Consolas
	args.socketServerConsola = abrirSocketInetServer(config->ip_nucleo,config->puerto_nucleo_prog);
	if (args.socketServerConsola == -1)
	{
		perror ("Error al abrir servidor para Consolas");
		exit (-1);
	}

	pthread_t hilo1;
	pthread_create(&hilo1,NULL,(void*)handleConsolas,(void *)&args);
	pthread_t hilo2;
	pthread_create(&hilo2,NULL,(void*)handleCPUs,(void *)&args);

	while(1){

	}
}

void handleConsolas(void* arguments){
	arg_struct *args = arguments;
	t_log* logger = args->logger;
	int socketServidor;				/* Descriptor del socket servidor */
	int *socketCliente = args->consolaSockets;/* Descriptores de sockets con clientes */
	int numeroClientes = 0;			/* Número clientes conectados */
	fd_set descriptoresLectura;	/* Descriptores de interes para select() */
	//int buffer;							/* Buffer para leer de los socket */
	int maximo;							/* Número de descriptor más grande */
	int i;								/* Para bubles */

	socketServidor = args->socketServerConsola;

	/* Bucle infinito.
	 * Se atiende a si hay más clientes para conectar y a los mensajes enviados
	 * por los clientes ya conectados */
	while (1)
	{
		/* Cuando un cliente cierre la conexión, se pondrá un -1 en su descriptor
		 * de socket dentro del array socketCliente. La función compactaClaves()
		 * eliminará dichos -1 de la tabla, haciéndola más pequeña.
		 *
		 * Se eliminan todos los clientes que hayan cerrado la conexión */
		compactaClaves (socketCliente, &numeroClientes);

		/* Se inicializa descriptoresLectura */
		FD_ZERO (&descriptoresLectura);

		/* Se añade para select() el socket servidor */
		FD_SET (socketServidor, &descriptoresLectura);

		/* Se añaden para select() los sockets con los clientes ya conectados */
		for (i=0; i<numeroClientes; i++)
			FD_SET (socketCliente[i], &descriptoresLectura);

		/* Se el valor del descriptor más grande. Si no hay ningún cliente,
		 * devolverá 0 */
		maximo = dameMaximo (socketCliente, numeroClientes);

		if (maximo < socketServidor)
			maximo = socketServidor;

		/* Espera indefinida hasta que alguno de los descriptores tenga algo
		 * que decir: un nuevo cliente o un cliente ya conectado que envía un
		 * mensaje */
		select (maximo + 1, &descriptoresLectura, NULL, NULL, NULL);
		imprimirArraySockets(socketCliente,MAX_CONSOLAS);
		/* Se comprueba si algún cliente ya conectado ha enviado algo */
		for (i=0; i<numeroClientes; i++)
		{
			if (FD_ISSET (socketCliente[i], &descriptoresLectura))
			{
				Package package;
				/* Se lee lo enviado por el cliente y se escribe en pantalla */
				if(recieve_and_deserialize(&package,socketCliente[i]) > 0){
					log_debug(logger,"Consola %d envía [message code]: %d, [Mensaje]: %s\n", i+1, package.msgCode, package.message);
					if(package.msgCode==NEW_ANSISOP_PROGRAM){
						log_debug(logger,"Consola %d solicito el inicio de un nuevo programa.",i+1);
						comunicarCPU(args->cpuSockets);
					}
				}
				else
				{
					/* Se indica que el cliente ha cerrado la conexión y se
					 * marca con -1 el descriptor para que compactaClaves() lo
					 * elimine */
					log_info(logger,"Consola %d ha cerrado la conexión\n", i+1);
					socketCliente[i] = -1;
				}
			}
		}

		/* Se comprueba si algún cliente nuevo desea conectarse y se le
		 * admite */
		if (FD_ISSET (socketServidor, &descriptoresLectura))
			nuevoCliente (socketServidor, socketCliente, &numeroClientes, MAX_CPUS);
	}
}

void handleCPUs(void* arguments){
	arg_struct *args = arguments;
	t_log* logger = args->logger;
	int socketServidor;				/* Descriptor del socket servidor */
	int *socketCliente = args->cpuSockets;/* Descriptores de sockets con clientes */
	int numeroClientes = 0;			/* Número clientes conectados */
	fd_set descriptoresLectura;	/* Descriptores de interes para select() */
	//int buffer;							/* Buffer para leer de los socket */
	int maximo;							/* Número de descriptor más grande */
	int i;								/* Para bubles */

	socketServidor = args->socketServerCPU;

	/* Bucle infinito.
	 * Se atiende a si hay más clientes para conectar y a los mensajes enviados
	 * por los clientes ya conectados */
	while (1)
	{
		/* Cuando un cliente cierre la conexión, se pondrá un -1 en su descriptor
		 * de socket dentro del array socketCliente. La función compactaClaves()
		 * eliminará dichos -1 de la tabla, haciéndola más pequeña.
		 *
		 * Se eliminan todos los clientes que hayan cerrado la conexión */
		compactaClaves (socketCliente, &numeroClientes);

		/* Se inicializa descriptoresLectura */
		FD_ZERO (&descriptoresLectura);

		/* Se añade para select() el socket servidor */
		FD_SET (socketServidor, &descriptoresLectura);

		/* Se añaden para select() los sockets con los clientes ya conectados */
		for (i=0; i<numeroClientes; i++)
			FD_SET (socketCliente[i], &descriptoresLectura);

		/* Se el valor del descriptor más grande. Si no hay ningún cliente,
		 * devolverá 0 */
		maximo = dameMaximo (socketCliente, numeroClientes);

		if (maximo < socketServidor)
			maximo = socketServidor;

		/* Espera indefinida hasta que alguno de los descriptores tenga algo
		 * que decir: un nuevo cliente o un cliente ya conectado que envía un
		 * mensaje */
		select (maximo + 1, &descriptoresLectura, NULL, NULL, NULL);

		/* Se comprueba si algún cliente ya conectado ha enviado algo */
		for (i=0; i<numeroClientes; i++)
		{
			if (FD_ISSET (socketCliente[i], &descriptoresLectura))
			{
				Package package;
				/* Se lee lo enviado por el cliente y se escribe en pantalla */
				//if ((leerSocket (socketCliente[i], (char *)&buffer, sizeof(int)) > 0))
				if(recieve_and_deserialize(&package,socketCliente[i]) > 0){
					log_debug(logger,"CPU %d envía [message code]: %d, [Mensaje]: %s\n", i+1, package.msgCode, package.message);
				}
				else
				{
					/* Se indica que el cliente ha cerrado la conexión y se
					 * marca con -1 el descriptor para que compactaClaves() lo
					 * elimine */
					log_info(logger,"CPU %d ha cerrado la conexión\n", i+1);
					socketCliente[i] = -1;
				}
			}
		}

		/* Se comprueba si algún cliente nuevo desea conectarse y se le
		 * admite */
		if (FD_ISSET (socketServidor, &descriptoresLectura))
			nuevoCliente (socketServidor, socketCliente, &numeroClientes, MAX_CPUS);
	}
}

int elegirRandomCPU(int cpuSockets[]){
	srand(time(NULL));
	int aux[MAX_CPUS];
	int i,j=0;
	for(i=0;i<MAX_CPUS;i++){
		if(cpuSockets[i]!=-1){
			aux[j]=cpuSockets[i];
			j++;
		}
	}
	int randNum = rand() % j;
	return aux[randNum];
}

void comunicarCPU(int cpuSockets[]){
	int socketCPU = elegirRandomCPU(cpuSockets);
	Package package;
	enviarMensajeCPU(&package,socketCPU);
}

void enviarMensajeCPU(Package* package,int socket){
	fillPackage(package,NEW_ANSISOP_PROGRAM,"3000");
	char* serializedPkg = serializarMensaje(package);
	escribirSocketServer(socket, (char *)serializedPkg, getLongitudPackage(package));
}

void imprimirArraySockets(int sockets[], int len){
	int i;
	printf("Sockets -> ");
	for(i=0;i<len;i++){
		printf("%d,",sockets[i]);
	}
	printf("\n");
}

void inicializarArraySockets(arg_struct* args){
	int i;
	for(i=0;i<MAX_CONSOLAS;i++){
		args->consolaSockets[i]=-1;
	}
	for(i=0;i<MAX_CPUS;i++){
		args->cpuSockets[i]=-1;
	}
}
