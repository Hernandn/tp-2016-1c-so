/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include "Nucleo.h"

void handleClients(Configuration* config){

	arg_struct args;
	args.config = config;
	args.socketServerUMC = -1;
	args.listaCPUs = list_create();

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
	logDebug("Planificador Server creado.");
	if (args.socketServerPlanificador == -1)
	{
		perror ("Error al abrir servidor para Threads");
		exit (-1);
	} else {
		logDebug("Conectando con Planificador Server.");
	}


	pthread_t hilo1;
	pthread_create(&hilo1,NULL,(void*)handleConsolas,(void *)&args);
	pthread_t hilo2;
	pthread_create(&hilo2,NULL,(void*)handleCPUs,(void *)&args);
	pthread_t hilo3;
	pthread_create(&hilo3,NULL,(void*)planificar,(void *)&args);

	while(1){
		//TODO: no se si hace falta esto
	}
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
		if(args->socketServerUMC==-1){
			args->socketClientPlanificador = conectarConPlanificador(PLANIFICADOR_IP,PLANIFICADOR_PORT);
		}
		if(args->socketServerUMC==-1){
			args->socketServerUMC = conectarConUMC(args->config);
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
						iniciarPrograma(estados,socketCliente[i],args->socketClientPlanificador,package->message);
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
				nuevoCPU(listaCPUs,socketCliente[numeroClientes-1],args->socketClientPlanificador);
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

	logDebug("Realizando handshake con UMC");
	//Se espera el handshake de la UMC para confirmar conexion
	package=malloc(sizeof(Package));
	if(recieve_and_deserialize(package, socket) > 0) {
		if(package->msgCode==HANDSHAKE_UMC) logDebug("Conexion con UMC confirmada");
	}

	//Le aviso a la UMC que soy un nucleo
	enviarMensajeSocket(socket,HANDSHAKE_NUCLEO,"");
	logDebug("Handshake con UMC exitoso!!");

	return socket;
}

void nuevoCPU(t_list* listaCPUs, int socketCPU, int socketPlanificador){
	CPU* nuevo = malloc(sizeof(CPU));
	nuevo->cpuFD = socketCPU;
	list_add(listaCPUs,nuevo);
	logTrace("Creado nuevo CPU: %d", socketCPU);
	liberarCPU(nuevo,socketPlanificador);
}

void destroyCPU(CPU* self){
	free(self);
}

//si se tiene el CPU
void liberarCPU(CPU* cpu, int socketPlanificador){
	cpu->libre = 1;	//true
	logTrace("Informando Planificador(%d) [CPU LIBRE]",socketPlanificador);
	informarPlanificador(socketPlanificador,CPU_LIBRE,0);
}

//si solo se tiene el socket del CPU
void liberarCPUporSocketFD(int socketCPU, arg_struct *args){
	CPU* cpu = buscarCPUporSocketFD(socketCPU,args->listaCPUs);
	if(cpu!=NULL){
		liberarCPU(cpu,args->socketClientPlanificador);
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
	logDebug("Planificador datos ip %s puerto %d.",ip,puerto);
	/* Se abre una conexión con el servidor */
	socket = abrirConexionInetConServer(ip, puerto);
	logDebug("Planificador conectado.");
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
		logTrace("CPU %d me informa que finalizo de ejecutar la instruccion");
		logTrace("Enviando al CPU la orden de Quantum Sleep");
		enviarMensajeSocket(socketCPU,QUANTUM_SLEEP_CPU,string_itoa(args->config->quantum_sleep));
	} else if(package->msgCode==QUANTUM_FINISHED){
		logTrace("CPU %d informa que finalizo 1 Quantum",socketCPU);
		quantumFinishedCallback(args->estados,atoi(package->message),args->config->quantum,socketCPU);
	} else if(package->msgCode==CPU_LIBRE){
		logTrace("CPU %d informa que esta Libre",socketCPU);
		liberarCPUporSocketFD(socketCPU,args);
	}
}

