/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include "Nucleo.h"
#include "planificador.h"

void handleClients(Configuration* config){

	arg_struct args;
	args.config = config;
	args.listaCPUs = list_create();

	//inicializo variables globales de sockets
	socketUMC = -1;
	socketPlanificador = -1;

	inicializarArraySockets(&args);

	Estados* estados = inicializarEstados();
	args.estados = estados;

	//abrir server para escuchar CPUs
	args.socketServerCPU = abrirSocketInetServer(config->ip_nucleo,config->puerto_nucleo_cpu);
	if (args.socketServerCPU == -1)
	{
		perror ("Error al abrir servidor para CPUs");
		exit (-1);
	}
	//abrir server para escuchar Consolas
	args.socketServerConsola = abrirSocketInetServer(config->ip_nucleo,config->puerto_nucleo_prog);
	if (args.socketServerConsola == -1)
	{
		perror ("Error al abrir servidor para Consolas");
		exit (-1);
	}
	//abrir server para escuchar mensajes enviados por los Threads anteriores
	args.socketServerPlanificador = abrirSocketInetServer(PLANIFICADOR_IP,PLANIFICADOR_PORT);
	if (args.socketServerPlanificador == -1)
	{
		perror ("Error al abrir servidor para Threads (Planificador)");
		exit (-1);
	}

	pthread_t hilo1;
	pthread_create(&hilo1,NULL,(void*)handleConsolas,(void *)&args);
	pthread_t hilo2;
	pthread_create(&hilo2,NULL,(void*)handleCPUs,(void *)&args);
	pthread_t hilo3;
	pthread_create(&hilo3,NULL,(void*)planificar,(void *)&args);

	pthread_join(hilo1,NULL);
	pthread_join(hilo2,NULL);
	pthread_join(hilo3,NULL);
}

void handleConsolas(void* arguments){
	arg_struct *args = arguments;
	Estados* estados = args->estados;
	int socketServidor;				/* Descriptor del socket servidor */
	int *socketCliente = args->consolaSockets;/* Descriptores de sockets con clientes */
	int numeroClientes = 0;			/* Número clientes conectados */
	fd_set descriptoresLectura;	/* Descriptores de interes para select() */
	//int buffer;							/* Buffer para leer de los socket */
	int maximo;							/* Número de descriptor más grande */
	int i;								/* Para bubles */

	socketServidor = args->socketServerConsola;

	/* Bucle infinito.
	 * Se atiende a si hay más clientes para conectar y a los mensajes enviados
	 * por los clientes ya conectados */
	while (1)
	{
		if(socketUMC==-1){
			socketUMC = conectarConUMC(args->config);
		}
		if(socketPlanificador==-1){
			socketPlanificador = conectarConPlanificador(PLANIFICADOR_IP,PLANIFICADOR_PORT);
			if(socketPlanificador!=-1){
				launch_IO_threads(estados);
			}
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
		imprimirArraySockets(socketCliente,MAX_CONSOLAS);
		/* Se comprueba si algún cliente ya conectado ha enviado algo */
		for (i=0; i<numeroClientes; i++)
		{
			if (FD_ISSET (socketCliente[i], &descriptoresLectura))
			{
				Package* package = malloc(sizeof(Package));
				/* Se lee lo enviado por el cliente y se escribe en pantalla */
				if(recieve_and_deserialize(package,socketCliente[i]) > 0){
					logDebug("Consola %d envía [message code]: %d, [Mensaje]: %s", i+1, package->msgCode, package->message);
					if(package->msgCode==NEW_ANSISOP_PROGRAM){
						logDebug("Consola %d solicito el inicio de un nuevo programa.",i+1);
						comunicarCPU(args->cpuSockets);
						iniciarPrograma(estados,socketCliente[i],package->message);
					}
					destroyPackage(package);
				}
				else
				{
					/* Se indica que el cliente ha cerrado la conexión y se
					 * marca con -1 el descriptor para que compactaClaves() lo
					 * elimine */
					logInfo("Consola %d ha cerrado la conexión.", i+1);
					abortarPrograma(estados,socketCliente[i]);
					socketCliente[i] = -1;
				}
			}
		}

		/* Se comprueba si algún cliente nuevo desea conectarse y se le
		 * admite */
		if (FD_ISSET (socketServidor, &descriptoresLectura)){
			nuevoCliente (socketServidor, socketCliente, &numeroClientes, MAX_CONSOLAS);
		}

	}
}

void handleCPUs(void* arguments){
	arg_struct *args = arguments;
	t_list* listaCPUs = args->listaCPUs;
	int socketServidor;				/* Descriptor del socket servidor */
	int *socketCliente = args->cpuSockets;/* Descriptores de sockets con clientes */
	int numeroClientes = 0;			/* Número clientes conectados */
	fd_set descriptoresLectura;	/* Descriptores de interes para select() */
	//int buffer;							/* Buffer para leer de los socket */
	int maximo;							/* Número de descriptor más grande */
	int i;								/* Para bubles */

	socketServidor = args->socketServerCPU;

	/* Bucle infinito.
	 * Se atiende a si hay más clientes para conectar y a los mensajes enviados
	 * por los clientes ya conectados */
	while (1)
	{
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
				Package* package = malloc(sizeof(Package));
				/* Se lee lo enviado por el cliente y se escribe en pantalla */
				//if ((leerSocket (socketCliente[i], (char *)&buffer, sizeof(int)) > 0))
				if(recieve_and_deserialize(package,socketCliente[i]) > 0){
					logDebug("CPU %d envía [message code]: %d, [Mensaje]: %s", i+1, package->msgCode, package->message);
					analizarMensajeCPU(socketCliente[i],package,args);
					destroyPackage(package);
				}
				else
				{
					/* Se indica que el cliente ha cerrado la conexión y se
					 * marca con -1 el descriptor para que compactaClaves() lo
					 * elimine */
					logInfo("CPU %d ha cerrado la conexión", i+1);
					eliminarCPU(listaCPUs,socketCliente[i]);
					socketCliente[i] = -1;
				}
			}
		}

		/* Se comprueba si algún cliente nuevo desea conectarse y se le
		 * admite */
		if (FD_ISSET (socketServidor, &descriptoresLectura)){
			int numAnterior = numeroClientes;
			nuevoCliente (socketServidor, socketCliente, &numeroClientes, MAX_CPUS);
			if(numeroClientes > numAnterior){//nuevo CPU aceptado
				nuevoCPU(listaCPUs,socketCliente[numeroClientes-1]);
			}
		}
	}
}

int elegirRandomCPU(int cpuSockets[]){
	srand(time(NULL));
	int aux[MAX_CPUS];
	int i,j=0;
	for(i=0;i<MAX_CPUS;i++){
		if(cpuSockets[i]!=-1){
			aux[j]=cpuSockets[i];
			j++;
		}
	}
	if(j>0){//para evitar la division por 0 si no hay CPUs
		int randNum = rand() % j;
		return aux[randNum];
	} else {
		return -1;
	}
}

void comunicarCPU(int cpuSockets[]){
	int socketCPU = elegirRandomCPU(cpuSockets);
	if(socketCPU!=-1){
		enviarMensajeSocket(socketCPU,NEW_ANSISOP_PROGRAM,"3000");
	}
}

void imprimirArraySockets(int sockets[], int len){
	int i;
	printf("Sockets -> ");
	for(i=0;i<len;i++){
		printf("%d,",sockets[i]);
	}
	printf("\n");
}

void inicializarArraySockets(arg_struct* args){
	int i;
	for(i=0;i<MAX_CONSOLAS;i++){
		args->consolaSockets[i]=-1;
	}
	for(i=0;i<MAX_CPUS;i++){
		args->cpuSockets[i]=-1;
	}
}

int conectarConUMC(Configuration* config){

	int socket;		/* descriptor de conexión con el servidor */
	Package *package;

	/* Se abre una conexión con el servidor */
	socket = abrirConexionInetConServer(config->ip_umc, config->puerto_umc);
	if (socket < 1) {
		logDebug("UMC se encuentra desconectada.");
		return -1;
	} else {
		logDebug("Conexion con UMC satisfactoria.");
	}

	//Se espera el handshake de la UMC para confirmar conexion
	package=malloc(sizeof(Package));
	if(recieve_and_deserialize(package, socket) > 0) {
		if(package->msgCode==HANDSHAKE_UMC){
			config->size_pagina = atoi(package->message);//recibo el tamanio de pagina
			logDebug("Conexion con UMC confirmada, tamanio de pagina: %d",config->size_pagina);
		}
	}

	//Le aviso a la UMC que soy un nucleo
	enviarMensajeSocket(socket,HANDSHAKE_NUCLEO,"");

	return socket;
}

void nuevoCPU(t_list* listaCPUs, int socketCPU){
	CPU* nuevo = malloc(sizeof(CPU));
	nuevo->cpuFD = socketCPU;
	list_add(listaCPUs,nuevo);
	logTrace("Creado nuevo CPU: %d", socketCPU);
	liberarCPU(nuevo);
}

void destroyCPU(CPU* self){
	free(self);
}

//si se tiene el CPU
void liberarCPU(CPU* cpu){
	cpu->libre = 1;	//true
	logTrace("Informando Planificador(%d) [CPU LIBRE]",socketPlanificador);
	informarPlanificador(CPU_LIBRE,0);
}

//si solo se tiene el socket del CPU
void liberarCPUporSocketFD(int socketCPU, arg_struct *args){
	CPU* cpu = buscarCPUporSocketFD(socketCPU,args->listaCPUs);
	if(cpu!=NULL){
		liberarCPU(cpu);
	}
}

CPU* buscarCPUporSocketFD(int socketCPU, t_list* listaCPUs){
	CPU* cpu;
	int i;
	for(i=0; i<listaCPUs->elements_count; i++){
		cpu = list_get(listaCPUs,i);
		if(cpu->cpuFD==socketCPU){
			return cpu;
		}
	}
	return NULL;
}

void eliminarCPU(t_list* listaCPUs,int socketCPU){
	int i;
	CPU* aEliminar = NULL;
	for(i=0;i<listaCPUs->elements_count;i++){
		aEliminar = list_get(listaCPUs,i);
		if(aEliminar->cpuFD==socketCPU){
			list_remove_and_destroy_element(listaCPUs,i,(void*)destroyCPU);
		}
	}
}

int conectarConPlanificador(char* ip, int puerto){

	int socket;		/* descriptor de conexión con el servidor */
	int buffer;		/* buffer de lectura de datos procedentes del servidor */
	int error;		/* error de lectura por el socket */
	/* Se abre una conexión con el servidor */
	socket = abrirConexionInetConServer(ip, puerto);
	/* Se lee el número de cliente, dato que nos da el servidor.*/
	error = leerSocketClient(socket, (char *)&buffer, sizeof(int));

	/* Si ha habido error de lectura lo indicamos y salimos */
	if (error < 1)
	{
		logDebug("Planificador se encuentra desconectado.");
	} else {
		logDebug("Conexion con Planificador satisfactoria.");
	}
	return socket;
}

void analizarMensajeCPU(int socketCPU , Package* package, arg_struct *args){
	if(package->msgCode==EXECUTION_FINISHED){
		logTrace("CPU %d me informa que finalizo de ejecutar la instruccion",socketCPU);
		logTrace("Enviando al CPU la orden de Quantum Sleep");
		enviarMensajeSocket(socketCPU,QUANTUM_SLEEP_CPU,string_itoa(args->config->quantum_sleep));
	} else if(package->msgCode==QUANTUM_FINISHED){
		logTrace("CPU %d informa que finalizo 1 Quantum",socketCPU);
		quantumFinishedCallback(args->estados,atoi(package->message),args->config->quantum,socketCPU);
	} else if(package->msgCode==PROGRAM_FINISHED){
		liberarCPUporSocketFD(socketCPU,args);
		PCB* pcbActualizado = deserializar_PCB(package->message);
		int socketConsola = getFromEXEC(args->estados,pcbActualizado->processID)->consolaFD;
		finalizarPrograma(args->estados,pcbActualizado,socketCPU);
		borrarSocketConsola(args,socketConsola);
	} else if(package->msgCode==CONTEXT_SWITCH_FINISHED){
		liberarCPUporSocketFD(socketCPU,args);
		PCB* pcbActualizado = deserializar_PCB(package->message);
		contextSwitchFinishedCallback(args->estados,pcbActualizado);
	} else if(package->msgCode==CPU_LIBRE){
		logTrace("CPU %d informa que esta Libre",socketCPU);
		liberarCPUporSocketFD(socketCPU,args);
	} else if(package->msgCode==EXEC_IO_OPERATION){
		logTrace("Solicitada operacion I/O");
		solicitud_io* solicitud = deserializar_ejecutarOperacionIO(package->message);
		atenderSolicitudDispositivoIO(args->estados,solicitud);
	} else if(package->msgCode==PRINT_VARIABLE){
		print_var* print = deserializar_imprimirVariable(package->message);
		int socketConsola = getFromEXEC(args->estados,print->pid)->consolaFD;
		char* serialized = serializar_imprimirVariable_consola(print->valor);
		enviarMensajeSocketConLongitud(socketConsola,PRINT_VARIABLE,serialized,sizeof(uint32_t));
		destroy_print_var(print);
		free(serialized);
	} else if(package->msgCode==PRINT_TEXT){
		print_text* print = deserializar_imprimirTexto(package->message);
		int socketConsola = getFromEXEC(args->estados,print->pid)->consolaFD;
		enviarMensajeSocket(socketConsola,PRINT_TEXT,print->text);
		destroy_print_text(print);
	} else if(package->msgCode==CPU_SIGNAL_DISCONNECTED){
		PCB* pcbActualizado = deserializar_PCB(package->message);
		contextSwitchFinishedCallback(args->estados,pcbActualizado);
	}
}

int getSocketUMC(){
	return socketUMC;
}

void borrarSocketConsola(arg_struct *args, int socketConsola){
	int i;
	for(i=0; i<MAX_CONSOLAS; i++){
		if(args->consolaSockets[i]==socketConsola){
			args->consolaSockets[i]=-1;
		}
	}
}


