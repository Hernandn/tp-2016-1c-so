/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include "funciones.h"

memoria memoria_principal;
tableRow* tabla;
static int socket_swap = -1;
static pthread_mutex_t
	comunicacion_swap_mutex,
	socket_swap_mutex;

void handleClients(Configuration* config){

	int socketServidor,						/* Descriptor del socket servidor */
		socket_cliente,						//Se usa para recivir al conexion y se envia al thread que la va a manejar
		numero_cpus = 0;					//Lleva la cuenta de las CPU's conectadas
	pthread_t threads_cpus[MAX_CLIENTES];	//Arrar de threads de cpu's
	t_arg_thread_cpu* arg_thread_cpu;		//Arguemntos para el thread del nuevo cpu
	Package* package=malloc(sizeof(Package));

	//Mutex para comunicacion con swap
	pthread_mutex_init(&comunicacion_swap_mutex,NULL);
	pthread_mutex_init(&socket_swap_mutex,NULL);

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
		/*Compruebo si se conecto el proceso Swap, no hace nada si ya estaba linkeado.
		 * Uso mutex porque socket_swap ahora es global.
		 */
		pthread_mutex_lock(&socket_swap_mutex);
		if(socket_swap==-1){
			socket_swap = conectarConSwap(config);
		}
		pthread_mutex_unlock(&socket_swap_mutex);

		//Acepto las conexiones y las mando a diferentes threads
		socket_cliente = aceptarConexionCliente(socketServidor);

		//Comienzo el handshake
		enviarMensajeSocket(socket_cliente,HANDSHAKE_UMC,"");

		//Espero respuesta y creo thread correspondiente
		if(recieve_and_deserialize(package,socket_cliente) > 0){
			logDebug("Mensaje recibido de %d",socket_cliente);
			switch(package->msgCode){
				case -1:
					logDebug("Se desconeccto el cliente %d",socket_cliente);
					break;
				case HANDSHAKE_CPU:
					logDebug("Cliente %d es un CPU"),socket_cliente;

					//TODO No estoy liberando esto en ningun momento.
					//Argumentos para el thread de la CPU. Mando una estructura porque es mas facil de modificar en un futuro.
					arg_thread_cpu = malloc(sizeof(t_arg_thread_cpu));
					arg_thread_cpu->socket_cpu=socket_cliente;

					/*TODO Entiendo para este punto deberia estar conectado el nucleo, sino no deberia poder
					 * conectar CPU's. ¿O no es necesario?
					 */
					pthread_create(&threads_cpus[numero_cpus],NULL,(void*) handle_cpu,(void*) arg_thread_cpu);
					numero_cpus++;	//TODO nunca estoy decrementando esta variable, eventualmente va a romper
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

void comunicarSWAP(int socket_swap, int accion){

	//Bloqueo la comunicacion con swap
	pthread_mutex_lock(&comunicacion_swap_mutex);
	pthread_mutex_lock(&socket_swap_mutex);

	if(accion==ALMACENAR_PAGINA_SWAP){
		enviarMensajeSocket(socket_swap,accion,"150,200,256");
	}

	//Libero la comunicacion con swap
	pthread_mutex_unlock(&socket_swap_mutex);
	pthread_mutex_unlock(&comunicacion_swap_mutex);
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
		*socket_cpu = &argumentos->socket_cpu;	//Lo guardo en variables para que sea mas comodo de usar
	Package* package;

	while(sigue){
		 package = malloc(sizeof(Package));
		if(recieve_and_deserialize(package,*socket_cpu) > 0){
			logDebug("CPU envía [message code]: %d, [Mensaje]: %s\n", package->msgCode, package->message);
			if(package->msgCode==INIT_PROGRAM){
				comunicarSWAP(socket_swap,ALMACENAR_PAGINA_SWAP);
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
