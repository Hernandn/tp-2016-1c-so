/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <mllibs/sockets/client.h>
#include <mllibs/sockets/package.h>
#include "CPU.h"

void conectarConUMC(){
	int socket;		/* descriptor de conexión con el servidor */
	int buffer;		/* buffer de lectura de datos procedentes del servidor */
	int error;		/* error de lectura por el socket */

	/* Se abre una conexión con el servidor */
	socket = abrirConexionInetConServer("127.0.0.1", 6690);

	/* Se lee el número de cliente, dato que nos da el servidor. Se escribe
	 * dicho número en pantalla.*/
	error = leerSocketClient(socket, (char *)&buffer, sizeof(int));

	/* Si ha habido error de lectura lo indicamos y salimos */
	if (error < 1)
	{
		printf ("Me han cerrado la conexión\n");
		exit(-1);
	}

	/* Se escribe el número de cliente que nos ha enviado el servidor */
	printf ("Soy el CPU %d\n", buffer);

	/* Bucle infinito. Envia al servidor el número de cliente y espera un
	 * segundo */
	Package package;
	while (1)
	{
		fillPackage(&package,SOLICITAR_BYTES_PAGINA,"20,200,64");
		//escribirSocket(socket, (char *)&buffer, sizeof(int));
		char* serializedPkg = serializarMensaje(&package);
		escribirSocketClient(socket, (char *)serializedPkg, getLongitudPackage(&package));

		sleep(3);
	}
}

void conectarConNucleo(){
	int socket;		/* descriptor de conexión con el servidor */
	int buffer;		/* buffer de lectura de datos procedentes del servidor */
	int error;		/* error de lectura por el socket */

	/* Se abre una conexión con el servidor */
	socket = abrirConexionInetConServer("127.0.0.1", 6700);

	/* Se lee el número de cliente, dato que nos da el servidor. Se escribe
	 * dicho número en pantalla.*/
	error = leerSocketClient(socket, (char *)&buffer, sizeof(int));

	/* Si ha habido error de lectura lo indicamos y salimos */
	if (error < 1)
	{
		printf ("Me han cerrado la conexión\n");
		exit(-1);
	}

	/* Se escribe el número de cliente que nos ha enviado el servidor */
	printf ("Soy el CPU %d\n", buffer);

	/* Bucle infinito. Envia al servidor el número de cliente y espera un
	 * segundo */
	Package package;
	while (1)
	{
		fillPackage(&package,SOLICITAR_BYTES_PAGINA,"15,500,128");
		//escribirSocket(socket, (char *)&buffer, sizeof(int));
		char* serializedPkg = serializarMensaje(&package);
		escribirSocketClient(socket, (char *)serializedPkg, getLongitudPackage(&package));

		sleep(3);
	}
}

