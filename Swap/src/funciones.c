/*
 * funciones.c
 *
 *  Created on: 22/04/2016
 *      Author: hernan
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <mllibs/sockets/client.h>
#include <mllibs/sockets/server.h>
#include <mllibs/sockets/package.h>
#include <commons/log.h>
#include <commons/string.h>
#include <mllibs/log/logger.h>
#include "Swap.h"

void handleUMCRequests(Configuration* config){
	int socketServidor;				/* Descriptor del socket servidor */
	int socketUMC[1];/* Descriptores de sockets con clientes */
	int numeroClientes = 0;			/* Número clientes conectados */
	fd_set descriptoresLectura;	/* Descriptores de interes para select() */
	//int buffer;							/* Buffer para leer de los socket */
	int maximo;							/* Número de descriptor más grande */

	//inicializo socketUMC
	socketUMC[0]=-1;
	/* Se abre el socket servidor, avisando por pantalla y saliendo si hay
	 * algún problema */
	socketServidor = abrirSocketInetServer(config->ip_swap,config->puerto_swap);

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
		/* Se inicializa descriptoresLectura */
		FD_ZERO (&descriptoresLectura);

		/* Se añade para select() el socket servidor */
		FD_SET (socketServidor, &descriptoresLectura);

		/* Se añaden para select() los sockets con los clientes ya conectados */
		logDebug("sockets %d",socketUMC[0]);
		if(socketUMC[0]!=-1){
			FD_SET (socketUMC[0], &descriptoresLectura);
		}

		maximo = socketUMC[0];

		if (maximo < socketServidor)
			maximo = socketServidor;

		logDebug("Esperando conexion");
		/* Espera indefinida hasta que alguno de los descriptores tenga algo
		 * que decir: un nuevo cliente o un cliente ya conectado que envía un
		 * mensaje */
		select (maximo + 1, &descriptoresLectura, NULL, NULL, NULL);

		if(socketUMC[0]!=-1){
			if (FD_ISSET (socketUMC[0], &descriptoresLectura))
			{
				Package* package = malloc(sizeof(Package));
				/* Se lee lo enviado por el cliente y se escribe en pantalla */
				//if ((leerSocket (socketCliente[i], (char *)&buffer, sizeof(int)) > 0))
				if(recieve_and_deserialize(package,socketUMC[0]) > 0){
					logDebug("UMC envía [message code]: %d, [Mensaje]: %s", package->msgCode, package->message);
					analizarMensaje(package,socketUMC[0],config);
					destroyPackage(package);
				}
				else
				{
					/* Se indica que el cliente ha cerrado la conexión y se
					 * marca con -1 el descriptor para que compactaClaves() lo
					 * elimine */
					logInfo("La UMC ha cerrado la conexión");
					socketUMC[0] = -1;
					numeroClientes = 0;
				}
			}
		}


		/* Se comprueba si algún cliente nuevo desea conectarse y se le
		 * admite */
		if (FD_ISSET (socketServidor, &descriptoresLectura))
			nuevoCliente (socketServidor, socketUMC, &numeroClientes, 2);
	}
}

void analizarMensaje(Package* package, int socketUMC, Configuration* config){

	if(package->msgCode==ALMACENAR_PAGINA_SWAP){
		logDebug("La UMC me solicito el almacenamiento de una nueva pagina.");
		//escribirPaginaDeProceso(pid,paginaNrom,pagina);
	} else if(package->msgCode==SOLICITAR_PAGINA_SWAP){
		char** params = string_split(package->message,",");
		//param 0: PID , param 1: numero de pagina
		pagina pag = leerPaginaDeProceso(atoi(params[0]),atoi(params[1]));
		//falta terminar, no darle bola
	} else if(package->msgCode==ALMACENAR_NUEVO_PROGRAMA_SWAP){
		//int frame = getFirstAvailableBlock(cantidadPaginas);
		//guardarPrograma(frame,pid,cantidadPaginas,paginas);
	} else if(package->msgCode==ELIMINAR_PROGRAMA_SWAP){
		eliminarPrograma(atoi(package->message));
	}
}

