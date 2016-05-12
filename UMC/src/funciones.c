/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include "funciones.h"

memoria memoria_principal;
tableRow* tabla;

void handleClients(Configuration* config){

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
			socketSwap = conectarConSwap(config);
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
					logDebug("Cliente %d envía [message code]: %d, [Mensaje]: %s", i+1, package.msgCode, package.message);
					if(package.msgCode==INIT_PROGRAM){
						comunicarSWAP(socketSwap,ALMACENAR_PAGINA_SWAP);
						logDebug("Se ha solicitado la inicializacion de un nuevo programa.");
					}
				} else {
					/* Se indica que el cliente ha cerrado la conexión y se
					 * marca con -1 el descriptor para que compactaClaves() lo
					 * elimine */
					logInfo("Cliente %d ha cerrado la conexión", i+1);
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
	if(accion==ALMACENAR_PAGINA_SWAP){
		Package package;
		fillPackage(&package,ALMACENAR_PAGINA_SWAP,"150,200,256");
		char* serializedPkg = serializarMensaje(&package);
		escribirSocketClient(socketSWAP, (char *)serializedPkg, getLongitudPackage(&package));
	}
}

int conectarConSwap(Configuration* config){

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
		logDebug("SWAP se encuentra desconectada.");
	} else {
		logDebug("Conexion con SWAP satisfactoria.");
	}
	return socket;
}

void inicializarUMC(Configuration* config){

	logDebug("Inicializando la UMC\n");

	tabla = crearTablaDePaginas(config->cantidad_paginas);
	memoria_principal=malloc(config->cantidad_paginas*config->size_pagina);
	logDebug("Creando memoria prinsipal de tamanio %d\n", config->cantidad_paginas*config->size_pagina);
}

tableRow* crearTablaDePaginas(int cantidadFrames){
	tableRow* table = malloc(sizeof(tableRow)*cantidadFrames);
	logDebug("Creando tabla de paginas con %d paginas\n",cantidadFrames);
	return table;
}
