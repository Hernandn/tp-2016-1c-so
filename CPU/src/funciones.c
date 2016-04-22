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
#include <commons/config.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <mllibs/sockets/client.h>
#include <mllibs/sockets/server.h>
#include <mllibs/sockets/package.h>
#include <commons/log.h>
#include "CPU.h"
#include "configuration.h"

Configuration* configurar(t_log* logger){

	Configuration* config = malloc(sizeof(Configuration));

	t_config* nConfig = config_create(CPU_CONFIG_PATH);
	if(nConfig==NULL){
		log_error(logger,"No se encontro el archivo de configuracion.");
		exit (1);
	}
	config->puerto_nucleo=config_get_int_value(nConfig,PUERTO_NUCLEO);
	config->ip_nucleo = config_get_string_value(nConfig,IP_NUCLEO);
	config->puerto_umc=config_get_int_value(nConfig,PUERTO_UMC);
	config->ip_umc = config_get_string_value(nConfig,IP_UMC);

	return config;
}

void conectarConUMC(void* arguments){
	arg_struct *args = arguments;
	t_log* logger = args->logger;
	int socket;		/* descriptor de conexión con el servidor */
	int buffer;		/* buffer de lectura de datos procedentes del servidor */
	int error;		/* error de lectura por el socket */

	/* Se abre una conexión con el servidor */
	socket = abrirConexionInetConServer(args->config->ip_umc, args->config->puerto_umc);

	/* Se lee el número de cliente, dato que nos da el servidor. Se escribe
	 * dicho número en pantalla.*/
	error = leerSocketClient(socket, (char *)&buffer, sizeof(int));

	/* Si ha habido error de lectura lo indicamos y salimos */
	if (error < 1)
	{
		log_error(logger,"Me han cerrado la conexión\n");
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

void conectarConNucleo(void* arguments){
	arg_struct *args = arguments;
	t_log* logger = args->logger;
	int socket;		/* descriptor de conexión con el servidor */
	int buffer;		/* buffer de lectura de datos procedentes del servidor */
	int error;		/* error de lectura por el socket */

	/* Se abre una conexión con el servidor */
	socket = abrirConexionInetConServer(args->config->ip_nucleo, args->config->puerto_nucleo);

	/* Se lee el número de cliente, dato que nos da el servidor. Se escribe
	 * dicho número en pantalla.*/
	error = leerSocketClient(socket, (char *)&buffer, sizeof(int));

	/* Si ha habido error de lectura lo indicamos y salimos */
	if (error < 1)
	{
		log_error(logger,"Me han cerrado la conexión\n");
		exit(-1);
	}

	/* Se escribe el número de cliente que nos ha enviado el servidor */
	log_info(logger,"Soy el CPU %d\n", buffer);

	Package package;
	while (1)
	{
		log_debug(logger,"Enviando mensaje al Nucleo.\n");
		fillPackage(&package,SOLICITAR_BYTES_PAGINA,"15,500,128");
		char* serializedPkg = serializarMensaje(&package);
		escribirSocketClient(socket, (char *)serializedPkg, getLongitudPackage(&package));
		if(recieve_and_deserialize(&package,socket) > 0){
			log_debug(logger,"Nucleo envía [message code]: %d, [Mensaje]: %s\n", package.msgCode, package.message);
			if(package.msgCode==NEW_ANSISOP_PROGRAM){
				log_debug(logger,"El Nucleo me comunica que se creo un programa nuevo.");
			}
		}

		sleep(3);
	}
}

