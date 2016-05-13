/*
 * funciones.c
 *
 *  Created on: 22/04/2016
 *      Author: hernan
 */

#include "funciones.h"

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


	//handshake con Nucleo
	logDebug("Iniciando Handshake con Nucleo.");
	handshake(socket);

	//iniciar programa
	logDebug("Iniciando programa AnSISOP.");
	iniciarProgramaAnsisop(socket,arch_programa);

	//simular ejecucion de programa
	while (1)
	{
		/*
		logDebug("Enviando mensaje al Nucleo.");
		fillPackage(&package,ANSISOP_PROGRAM,"20,200,64");

		char* serializedPkg = serializarMensaje(&package);
		escribirSocketClient(socket, (char *)serializedPkg, getLongitudPackage(&package));

		sleep(3);
		*/
	}

	close(socket);
}

void handshake(int serverSocket){
	enviarMensajeSocket(serverSocket,HANDSHAKE,"2000");
}

void iniciarProgramaAnsisop(int serverSocket,char* arch_programa){
	enviarMensajeSocket(serverSocket,NEW_ANSISOP_PROGRAM,obtener_programa(arch_programa));
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

	//Obtengo el tamanio del archivo
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	//Cargo el archivo en el buffer
	programa = malloc(fsize + 1);
	fread(programa, fsize, 1, fp);
	fclose(fp);

	//Agrego el caracter de fin al buffer
	programa[fsize] = 0;

	logDebug("Programa leido: \n%s",programa);

	return programa;
}
