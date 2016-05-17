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

		int pid = getProcessID_EscribirPagina(package->message);
		int numeroPagina = getNumeroPagina_EscribirPagina(package->message);
		pagina pagina = getPagina_EscribirPagina(package->message,config->size_pagina);
		escribirPaginaDeProceso(pid,numeroPagina,pagina);
		logDebug("Escritura: [PID: %d, Pagina: %d]",pid,numeroPagina);

	} else if(package->msgCode==SOLICITAR_PAGINA_SWAP){

		int pid = getProcessID_SolicitarPagina(package->message);
		int numeroPagina = getNumeroPagina_SolicitarPagina(package->message);
		pagina page = leerPaginaDeProceso(pid,numeroPagina);
		enviarMensajeSocketConLongitud(socketUMC,SOLICITAR_PAGINA_SWAP,page,config->size_pagina);
		logDebug("Lectura: [PID: %d, Pagina: %d]",pid,numeroPagina);

	} else if(package->msgCode==ALMACENAR_NUEVO_PROGRAMA_SWAP){

		logDebug("Message long: %d",package->message_long);
		int pid = getProcessID_NuevoPrograma(package->message);
		int cantidadPaginas = getCantidadPaginas_NuevoPrograma(package->message);

		logDebug("Programa PID:%d , cant paginas: %d",pid,cantidadPaginas);
		int frame = getFirstAvailableBlock(cantidadPaginas);

		if(frame>=0){//hay espacio disponible
			pagina* paginas = getPaginas_NuevoPrograma(package->message,cantidadPaginas,config->size_pagina);
			guardarPrograma(frame,pid,cantidadPaginas,paginas);
			logDebug("Programa PID:%d se ha almacenado en Swap (%d pags)",pid,cantidadPaginas);
		} else if(frame==-1){
			logInfo("No hay espacio suficiente en Swap para almacenar el programa PID:%d",pid);
		} else if(frame==-2){
			logInfo("No hay espacio suficiente en Swap para almacenar el programa PID:%d, pero se puede realizar una Compactacion",pid);
		}

	} else if(package->msgCode==ELIMINAR_PROGRAMA_SWAP){

		int pid = atoi(package->message);
		eliminarPrograma(pid);
		logDebug("Programa PID:%d ha sido eliminado",pid);

	}
}


int getProcessID_NuevoPrograma(char* str){
	return *str;
}

int getCantidadPaginas_NuevoPrograma(char* str){
	return *(str+sizeof(uint32_t));
}

pagina* getPaginas_NuevoPrograma(char* str, int cantPags, int size){
	int offset = sizeof(uint32_t)*2;
	pagina* pags = malloc(sizeof(pagina)*cantPags);
	int i;
	for(i=0; i<cantPags; i++){
		pags[i] = malloc(sizeof(char)*size);
		memcpy(pags[i],str+offset,size);
		offset+=size;
	}
	return pags;
}

int getProcessID_EscribirPagina(char* str){
	return *str;
}

int getNumeroPagina_EscribirPagina(char* str){
	return *(str+sizeof(uint32_t));
}

pagina getPagina_EscribirPagina(char* str, int size){
	int offset = sizeof(uint32_t)*2;
	pagina pag = malloc(sizeof(char)*size);
	memcpy(pag,str+offset,size);
	return pag;
}

int getProcessID_SolicitarPagina(char* str){
	return *str;
}

int getNumeroPagina_SolicitarPagina(char* str){
	return *(str+sizeof(uint32_t));
}
