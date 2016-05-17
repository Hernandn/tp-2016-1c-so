/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include "CPU.h"
#include "configuration.h"

Configuration* configurar(){

	Configuration* config = malloc(sizeof(Configuration));

	t_config* nConfig = config_create(CPU_CONFIG_PATH);
	if(nConfig==NULL){
		//para debuggear desde eclipse
		nConfig = config_create(CPU_CONFIG_PATH_ECLIPSE);
		if(nConfig==NULL){
			printf("No se encontro el archivo de configuracion.");
			exit (1);
		}
	}
	config->puerto_nucleo=config_get_int_value(nConfig,PUERTO_NUCLEO);
	config->ip_nucleo = config_get_string_value(nConfig,IP_NUCLEO);
	config->puerto_umc=config_get_int_value(nConfig,PUERTO_UMC);
	config->ip_umc = config_get_string_value(nConfig,IP_UMC);
	//configuracion de log
	config->log_level = config_get_string_value(nConfig,LOG_LEVEL);
	config->log_file = config_get_string_value(nConfig,LOG_FILE);
	config->log_program_name = config_get_string_value(nConfig,LOG_PROGRAM_NAME);
	config->log_print_console = config_get_int_value(nConfig,LOG_PRINT_CONSOLE);

	return config;
}

void conectarConUMC(void* arguments){
	arg_struct *args = arguments;
	int socket;		/* descriptor de conexión con el servidor */
	Package* package;

	/* Se abre una conexión con el servidor */
	socket = abrirConexionInetConServer(args->config->ip_umc, args->config->puerto_umc);
	if (socket < 1){
		logError("Me han cerrado la conexión.");
		exit(-1);
	}

	args->socketUMC = socket;

	logDebug("Realizando handshake con UMC");
	package=malloc(sizeof(Package));
	if(recieve_and_deserialize(package, socket) > 0) {
		if(package->msgCode==HANDSHAKE_UMC) logDebug("Conexion con UMC confirmada");
	}

	//Le aviso a la UMC que soy un nucleo
	enviarMensajeSocket(socket,HANDSHAKE_CPU,"");
	logDebug("Handshake con UMC exitoso!!");

	/* Bucle infinito. Envia al servidor el número de cliente y espera un
	 * segundo */
	//Package package;
	while (1)
	{
		/*
		fillPackage(&package,SOLICITAR_BYTES_PAGINA,"20,200,64");
		//escribirSocket(socket, (char *)&buffer, sizeof(int));
		char* serializedPkg = serializarMensaje(&package);
		escribirSocketClient(socket, (char *)serializedPkg, getLongitudPackage(&package));

		sleep(10);
		*/
	}
}

void conectarConNucleo(void* arguments){
	arg_struct *args = arguments;
	int buffer;		/* buffer de lectura de datos procedentes del servidor */
	int error;		/* error de lectura por el socket */

	/* Se abre una conexión con el servidor */
	args->socketNucleo = abrirConexionInetConServer(args->config->ip_nucleo, args->config->puerto_nucleo);

	/* Se lee el número de cliente, dato que nos da el servidor. Se escribe
	 * dicho número en pantalla.*/
	error = leerSocketClient(args->socketNucleo, (char *)&buffer, sizeof(int));

	/* Si ha habido error de lectura lo indicamos y salimos */
	if (error < 1)
	{
		logError("Me han cerrado la conexión.");
		exit(-1);
	}

	/* Se escribe el número de cliente que nos ha enviado el servidor */
	logDebug("Soy el CPU %d", buffer);


	while (1)
	{
		Package* package = malloc(sizeof(Package));
		if(recieve_and_deserialize(package,args->socketNucleo) > 0){
			logDebug("Nucleo envía [message code]: %d, [Mensaje]: %s", package->msgCode, package->message);
			analizarMensaje(package,args);
		}
		destroyPackage(package);
	}
}

void analizarMensaje(Package* package, arg_struct *args){
	if(package->msgCode==NEW_ANSISOP_PROGRAM){
		logDebug("El Nucleo me comunica que se creo un programa nuevo.");
		enviarMensajeSocket(args->socketUMC,INIT_PROGRAM,"INITPROGRAM");//envio mensaje a la UMC
	} else if(package->msgCode==EXEC_NEW_PROCESS){
		args->processID = atoi(package->message);	//actualizo al nuevo proceso recibido por mensaje
		ejecutarProceso(args,package);
	} else if(package->msgCode==QUANTUM_SLEEP_CPU){
		quantumSleep(args,atoi(package->message));
	} else if(package->msgCode==CONTINUE_EXECUTION){
		ejecutarProceso(args,package);
	} else if(package->msgCode==ABORT_EXECUTION){
		abortarProceso(args);
	}
}

void ejecutarProceso(arg_struct *args, Package* package){
	logTrace("Ejecutando instruccion del proceso PID:%d...",args->processID);
	//TODO: Aca iria el codigo a partir del cual empieza toda la ejecucion de una instruccion

	logTrace("Informando al Nucleo que finalizo la ejecucion de 1 Quantum...");
	enviarMensajeSocket(args->socketNucleo,EXECUTION_FINISHED,"");
}

void quantumSleep(arg_struct *args, int milisegundos){
	logTrace("Sleeping %d miliseconds (-.-) ...zzzZZZ",milisegundos);
	//TODO: hay que ver que funcion usar para que haga sleep de milisegundos
	sleep(milisegundos);
	enviarMensajeSocket(args->socketNucleo,QUANTUM_FINISHED,string_itoa(args->processID));
}

void abortarProceso(arg_struct *args){
	logTrace("Abortando proceso PID:%d",args->processID);
	//TODO: aca se debe ejecutar el context switch (actualizar registros, guardar el proceso en UMC)
	logTrace("Informando al Nucleo que el CPU se encuentra libre");
	enviarMensajeSocket(args->socketNucleo,CPU_LIBRE,"");
}



