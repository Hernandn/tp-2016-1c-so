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

	int socketSwap = -1,
		socketServidor,						/* Descriptor del socket servidor */
		socket_cliente,						//Se usa para recivir al conexion y se envia al thread que la va a manejar
		numero_cpus = 0;					//Lleva la cuenta de las CPU's conectadas
	pthread_t threads_cpus[MAX_CLIENTES];	//Arrar de threads de cpu's
	t_arg_thread_cpu* arg_thread_cpu;		//Arguemntos para el thread del nuevo cpu
	Package* package=malloc(sizeof(Package));
	char* serializedPkg;

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

		//Acepto las conexiones y las mando a diferentes threads
		socket_cliente = aceptarConexionCliente(socketServidor);
		package = fillPackage(HANDSHAKE_UMC,"");
		serializedPkg = serializarMensaje(package);
		escribirSocketClient(socket_cliente, (char *)serializedPkg, getLongitudPackage(package));
		escribirSocketServer(socket_cliente, (char *)&numero_cpus, sizeof(int));
		if(recieve_and_deserialize(package,socket_cliente) > 0){
			logDebug("Mensaje recibido de %d",socket_cliente);
			switch(package->msgCode){
				case HANDSHAKE_CPU:
					logDebug("Cliente %d es un CPU"),socket_cliente;
					/*TODO No estoy liberando esto en ningun momento. No se si hacerlo dentro
					 * del thread o si conviene guardarlo para poder camiar el socket swap en
					 * caliente sin que los threads se vean afectados. En caso de elegir la segunda
					 * opcion, cuando se actualice el socketSwap (porque se desconecto y reconecto
					 * con un identificador distinto por ejemplo) habria que actualizar todas las configuraiones.
					 */

					arg_thread_cpu = malloc(sizeof(t_arg_thread_cpu));
					arg_thread_cpu->socket_cpu=socket_cliente;
					arg_thread_cpu->socket_swap=socketSwap;

					/*TODO Para este punto deberia estar conectado el nucleo, sino no deberia poder
					 * conectar CPU's
					 */
					pthread_create(&threads_cpus[numero_cpus],NULL,(void*) handle_cpu,(void*) arg_thread_cpu);
					numero_cpus++;
					break;

				case HANDSHAKE_NUCLEO:
					logDebug("Cliente %d es un Nucleo",socket_cliente);
					//Por ahora entiendo que no recibe nada del nucleo asi que queda ahi.
					break;

				default:
					logDebug("El cliente %d no se identifico",socket_cliente);
					close(socket_cliente);
			}
		}
	}
}

void comunicarSWAP(int socketSWAP, int accion){
	if(accion==ALMACENAR_PAGINA_SWAP){
		enviarMensajeSocket(socketSWAP,accion,"150,200,256");
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

void handle_cpu(t_arg_thread_cpu* argumentos){
	int sigue = 1,	//Ya se, no es muy original que digamos
		*socket_cpu = &argumentos->socket_cpu,	//Lo guardo en variables para que sea mas comodo de usar
		*socket_swap = &argumentos->socket_swap;
	Package* package;

	while(sigue){
		 package = malloc(sizeof(Package));
		if(recieve_and_deserialize(package,*socket_cpu) > 0){
			logDebug("CPU envía [message code]: %d, [Mensaje]: %s\n", package->msgCode, package->message);
			if(package->msgCode==INIT_PROGRAM){
				comunicarSWAP(*socket_swap,ALMACENAR_PAGINA_SWAP);
				logDebug("Se ha solicitado la inicializacion de un nuevo programa.\n");
			}
		} else {
			//Si el cliente cerro la conexion se termino el thread
			sigue=0;
			logInfo("Cliente ha cerrado la conexión, cerrando thread\n");
		}
		destroyPackage(package);
	}
	logInfo("Fin thread\n");
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
