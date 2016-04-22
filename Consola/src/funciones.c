/*
 * funciones.c
 *
 *  Created on: 22/04/2016
 *      Author: hernan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <mllibs/sockets/client.h>
#include <mllibs/sockets/package.h>
#include "configuration.h"
#include "consola.h"

void comunicacionConNucleo(Configuration* config){

	int resp;
	int socket;
	int buffer;

	socket = abrirConexionInetConServer(config->ip_nucleo,config->puerto_nucleo);
	resp = leerSocketClient(socket, (char *)&buffer, sizeof(int));

	if (resp < 1)
	{
			printf ("Me han cerrado la conexión\n");
			exit(-1);
	}

	printf ("Soy la consola %d\n", buffer);

	Package package;

	//handshake con Nucleo
	handshake(&package,socket);

	//iniciar programa
	iniciarProgramaAnsisop(&package,socket);

	//simular ejecucion de programa
	while (1)
	{
		fillPackage(&package,ANSISOP_PROGRAM,"20,200,64");

		char* serializedPkg = serializarMensaje(&package);
		escribirSocketClient(socket, (char *)serializedPkg, getLongitudPackage(&package));

		sleep(3);
	}

	close(socket);
}

void handshake(Package* package,int serverSocket){
	fillPackage(&package,HANDSHAKE,"2000");
	char* serializedPkg = serializarMensaje(&package);
	escribirSocketClient(serverSocket, (char *)serializedPkg, getLongitudPackage(&package));
}

void iniciarProgramaAnsisop(Package* package,int serverSocket){
	fillPackage(&package,NEW_ANSISOP_PROGRAM,"2000");
	char* serializedPkg = serializarMensaje(&package);
	escribirSocketClient(serverSocket, (char *)serializedPkg, getLongitudPackage(&package));
}
