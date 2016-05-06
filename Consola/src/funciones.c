/*
 * funciones.c
 *
 *  Created on: 22/04/2016
 *      Author: hernan
 */

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

void comunicacionConNucleo(Configuration* config, char* arch_programa){

	logDebug("Iniciando comunicacion con Nucleo.");

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

	logInfo("Soy la consola %d\n",buffer);

	Package package;

	//handshake con Nucleo
	logDebug("Iniciando Handshake con Nucleo.");
	handshake(&package,socket);

	//iniciar programa
	logDebug("Iniciando programa AnSISOP.");
	iniciarProgramaAnsisop(&package,socket,arch_programa);

	//simular ejecucion de programa
	while (1)
	{
		logDebug("Enviando mensaje al Nucleo.");
		fillPackage(&package,ANSISOP_PROGRAM,"20,200,64");

		char* serializedPkg = serializarMensaje(&package);
		escribirSocketClient(socket, (char *)serializedPkg, getLongitudPackage(&package));

		sleep(3);
	}

	close(socket);
}

void handshake(Package* package,int serverSocket){
	fillPackage(package,HANDSHAKE,"2000");
	char* serializedPkg = serializarMensaje(package);
	escribirSocketClient(serverSocket, (char *)serializedPkg, getLongitudPackage(package));
}

void iniciarProgramaAnsisop(Package* package,int serverSocket,char* arch_programa){
	fillPackage(package,NEW_ANSISOP_PROGRAM,obtener_programa(arch_programa));
	char* serializedPkg = serializarMensaje(package);
	escribirSocketClient(serverSocket, (char *)serializedPkg, getLongitudPackage(package));
}

char* obtener_programa(char* arch_programa){
	FILE *fp;
	char* programa;
	long fsize;

	if((fp=fopen(arch_programa,"r"))==NULL){
		logError("Error al abrir el programa %s",arch_programa);
		exit (EXIT_FAILURE);
	}

	logDebug("Leyendo programa ansisop %s\n",arch_programa);

	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	programa = malloc(fsize + 1);
	fread(programa, fsize, 1, fp);
	fclose(fp);

	programa[fsize] = 0;

	logDebug("Programa leido: \n%s",programa);

	return programa;
}
