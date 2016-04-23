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
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include "UMC.h"
#include <mllibs/sockets/server.h>
#include <mllibs/sockets/client.h>
#include <mllibs/sockets/package.h>
#include "configuration.h"

Configuration* configurar(t_log* logger){

	Configuration* config = malloc(sizeof(Configuration));

	t_config* nConfig = config_create(UMC_CONFIG_PATH);
	if(nConfig==NULL){
		log_error(logger,"No se encontro el archivo de configuracion.");
		exit (1);
	}
	config->puerto_swap=config_get_int_value(nConfig,PUERTO_SWAP);
	config->ip_swap = config_get_string_value(nConfig,IP_SWAP);
	config->puerto_umc=config_get_int_value(nConfig,PUERTO_UMC);
	config->ip_umc = config_get_string_value(nConfig,IP_UMC);

	return config;
}

void handleClients(Configuration* config, t_log* logger){

	int socketSwap = -1;

	int socketServidor;				/* Descriptor del socket servidor */
	int socketCliente[MAX_CLIENTES];/* Descriptores de sockets con clientes */
	int numeroClientes = 0;			/* Número clientes conectados */
	fd_set descriptoresLectura;	/* Descriptores de interes para select() */
	//int buffer;							/* Buffer para leer de los socket */
	int maximo;							/* Número de descriptor más grande */
	int i;								/* Para bubles */

	/* Se abre el socket servidor, avisando por pantalla y saliendo si hay
	 * algún problema */
	socketServidor = abrirSocketInetServer(config->ip_umc,config->puerto_umc);
	if (socketServidor == -1)
	{
		perror ("Error al abrir servidor");
		exit (-1);
	}

	/* Bucle infinito.
	 * Se atiende a si hay más clientes para conectar y a los mensajes enviados
	 * por los clientes ya conectados */
	while (1)
	{
		//Compruebo si se conecto el proceso Swap
		//No hace nada si ya estaba linkeado
		if(socketSwap==-1){
			socketSwap = conectarConSwap(config,logger);
		}

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
					log_debug(logger,"Cliente %d envía [message code]: %d, [Mensaje]: %s\n", i+1, package.msgCode, package.message);
					if(package.msgCode==INIT_PROGRAM){
						comunicarSWAP(socketSwap,ALMACENAR_BYTES_PAGINA_SWAP);
						log_debug(logger,"Se ha solicitado la inicializacion de un nuevo programa.");
					}
				} else {
					/* Se indica que el cliente ha cerrado la conexión y se
					 * marca con -1 el descriptor para que compactaClaves() lo
					 * elimine */
					log_info(logger,"Cliente %d ha cerrado la conexión\n", i+1);
					socketCliente[i] = -1;
				}
			}
		}

		/* Se comprueba si algún cliente nuevo desea conectarse y se le
		 * admite */
		if (FD_ISSET (socketServidor, &descriptoresLectura))
			nuevoCliente (socketServidor, socketCliente, &numeroClientes, MAX_CLIENTES);
	}
}

void comunicarSWAP(int socketSWAP, int accion){
	if(accion==ALMACENAR_BYTES_PAGINA_SWAP){
		Package package;
		fillPackage(&package,ALMACENAR_BYTES_PAGINA_SWAP,"150,200,256");
		char* serializedPkg = serializarMensaje(&package);
		escribirSocketClient(socketSWAP, (char *)serializedPkg, getLongitudPackage(&package));
	}
}

int conectarConSwap(Configuration* config, t_log* logger){

	int socket;		/* descriptor de conexión con el servidor */
	int buffer;		/* buffer de lectura de datos procedentes del servidor */
	int error;		/* error de lectura por el socket */

	/* Se abre una conexión con el servidor */
	socket = abrirConexionInetConServer(config->ip_swap, config->puerto_swap);

	/* Se lee el número de cliente, dato que nos da el servidor.*/
	error = leerSocketClient(socket, (char *)&buffer, sizeof(int));

	/* Si ha habido error de lectura lo indicamos y salimos */
	if (error < 1)
	{
		log_debug(logger,"SWAP se encuentra desconectada.");
	} else {
		log_debug(logger,"Conexion con SWAP satisfactoria.");
	}
	return socket;
}

