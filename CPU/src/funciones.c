/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include "CPU.h"
#include "configuration.h"
#include "primitivas.h"
#include <mllibs/nucleoCpu/interfaz.h>

AnSISOP_funciones functions = {
		.AnSISOP_definirVariable		= ml_definirVariable,
		.AnSISOP_obtenerPosicionVariable= ml_obtenerPosicionVariable,
		.AnSISOP_dereferenciar			= ml_dereferenciar,
		.AnSISOP_asignar				= ml_asignar,
		.AnSISOP_obtenerValorCompartida = ml_obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida = ml_asignarValorCompartida,
		.AnSISOP_irAlLabel				= ml_irAlLabel,
		.AnSISOP_llamarSinRetorno		= ml_llamarSinRetorno,
		.AnSISOP_llamarConRetorno		= ml_llamarConRetorno,
		.AnSISOP_finalizar				= ml_finalizar,
		.AnSISOP_retornar				= ml_retornar,
		.AnSISOP_imprimir				= ml_imprimir,
		.AnSISOP_imprimirTexto			= ml_imprimirTexto,
		.AnSISOP_entradaSalida			= ml_entradaSalida,
};

AnSISOP_kernel kernel_functions = {
		.AnSISOP_wait	= ml_wait,
		.AnSISOP_signal = ml_signal,
};

//TODO: por ahora queda aca global para que lo usen las primitivas, hay que ver donde meterlo
int socketUMC;

PCB* pcbActual;//pcb del proceso que se esta ejecutando actualmente en el CPU

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

	socketUMC = socket;

	logDebug("Realizando handshake con UMC");
	package=malloc(sizeof(Package));
	if(recieve_and_deserialize(package, socket) > 0) {
		if(package->msgCode==HANDSHAKE_UMC){
			if(package->msgCode==HANDSHAKE_UMC){
				args->config->size_pagina = atoi(package->message);//recibo el tamanio de pagina
				logDebug("Conexion con UMC confirmada, tamanio de pagina: %d",args->config->size_pagina);
			}
		}
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
		enviarMensajeSocket(socketUMC,INIT_PROGRAM,"INITPROGRAM");//envio mensaje a la UMC
	} else if(package->msgCode==EXEC_NEW_PROCESS){
		cargarContextoPCB(package);
		ejecutarProceso(args);
	} else if(package->msgCode==QUANTUM_SLEEP_CPU){
		quantumSleep(args,atoi(package->message));
	} else if(package->msgCode==CONTINUE_EXECUTION){
		ejecutarProceso(args);
	} else if(package->msgCode==ABORT_EXECUTION){
		abortarProceso(args);
	} else if(package->msgCode==CONTEXT_SWITCH){
		contextSwitch(args);
	}
}

void contextSwitch(arg_struct *args){
	logTrace("Cambiando contexto proceso PID:%d",pcbActual->processID);
	//TODO: aca se debe ejecutar el context switch (actualizar registros, guardar el proceso en UMC)
	logTrace("Informando al Nucleo que el CPU se encuentra libre");
	informarNucleoContextSwitchFinished(args->socketNucleo,pcbActual);
}

void cargarContextoPCB(Package* package){
	pcbActual = deserializar_PCB(package->message);
	logTrace("Contexto de proceso cargado PID:%d...",pcbActual->processID);
}

void ejecutarProceso(arg_struct *args){
	logTrace("Ejecutando instruccion del proceso PID:%d...",pcbActual->processID);

	ejecutarInstruccion();

	logTrace("Informando al Nucleo que finalizo la ejecucion de 1 Quantum. (PC:%d/%d)",pcbActual->programCounter,pcbActual->codeIndex->instrucciones_size);
	enviarMensajeSocket(args->socketNucleo,EXECUTION_FINISHED,"");
}

void quantumSleep(arg_struct *args, int milisegundos){
	logTrace("Sleeping %d miliseconds (-.-) ...zzzZZZ",milisegundos);

	usleep(milisegundos*1000);//convierto micro en milisegundos

	if(programaFinalizado()){
		//envia el PCB de nuevo al Nucleo
		informarNucleoFinPrograma(args->socketNucleo,pcbActual);
		logTrace("CPU se encuentra libre");
	} else {
		informarNucleoQuantumFinished(args->socketNucleo,pcbActual);
	}
}

//verifica si se terminaron las instrucciones del programa
bool programaFinalizado(){
	return pcbActual->codeIndex->instrucciones_size - pcbActual->programCounter <= 0;
}

void abortarProceso(arg_struct *args){
	logTrace("Abortando proceso PID:%d",pcbActual->processID);
	//TODO: aca se debe ejecutar el context switch (actualizar registros, guardar el proceso en UMC)
	logTrace("Informando al Nucleo que el CPU se encuentra libre");
	informarNucleoCPUlibre(args->socketNucleo);
}

void ejecutarInstruccion(){
	char* instruccion = getSiguienteInstruccion();
	printf("=================================\n");
	printf("Ejecutando '%s'\n", instruccion);
	analizadorLinea(instruccion, &functions, &kernel_functions);
	printf("=================================\n");
	if(instruccion!=NULL){
		free(instruccion);
	}
	//incremento el PC cuando termina de ejecutar la instruccion
	pcbActual->programCounter++;
}


void analizarRespuestaUMC(){
	Package* package = malloc(sizeof(Package));
	if(recieve_and_deserialize(package,socketUMC) > 0){
		logDebug("UMC envía [message code]: %d, [Mensaje]: %s", package->msgCode, package->message);
	}
	destroyPackage(package);
}

int getSocketUMC(){
	return socketUMC;
}


char* getInstruccion(char* codigo, int offset, int length){
	char* instruccion = malloc(sizeof(char)*length+1);
	if(instruccion!=NULL){
		memcpy(instruccion,codigo+offset,length);
		instruccion[length]='\0';
	}
	return instruccion;
}

char* getSiguienteInstruccion(){
	int offset = pcbActual->codeIndex->instrucciones_serializado[pcbActual->programCounter].start;
	int length = pcbActual->codeIndex->instrucciones_serializado[pcbActual->programCounter].offset;
	return getInstruccion(pcbActual->programa,offset,length);
}



