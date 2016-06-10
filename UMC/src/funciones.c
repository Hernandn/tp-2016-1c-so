/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include "funciones.h"
#include "interfazSwap.h"

//----------------------------------PRIVADO---------------------------------------

static pthread_key_t key_pid;

//----------------------------------PUBLICO---------------------------------------

void handleClients(){


	int socketServidor,							//Descriptor del socket servidor
		socket_cliente;							//Se usa para recivir al conexion y se envia al thread que la va a manejar

	pthread_t thread_tmp;						//Variable Temporal para crear threads;
	t_arg_thread_cpu* arg_thread_cpu;			//Arguemntos para el thread del nuevo cpu
	t_arg_thread_nucleo* arg_thread_nucleo;		//Argumentos para el thread del nuevo Nucleo
	pthread_attr_t thread_detached_attr;		//Atributos para crear socket detached

	Package* package=malloc(sizeof(Package));

	//Mutex para comunicacion con swap
	pthread_mutex_init(&comunicacion_swap_mutex,NULL);
	pthread_mutex_init(&socket_swap_mutex,NULL);

	//Completo los atributos de thread para que sea detached. Se va a usar para los thread de CPU
	pthread_attr_init(&thread_detached_attr);
	pthread_attr_setdetachstate(&thread_detached_attr,PTHREAD_CREATE_DETACHED);

	//Creo la key para los pids de cpus
	pthread_key_create(&key_pid, NULL);

	/* Se abre el socket servidor, avisando por pantalla y saliendo si hay
	 * algún problema */
	socketServidor = abrirSocketInetServer(config->ip_umc,config->puerto_umc);
	if (socketServidor == -1)
	{
		perror ("Error al abrir servidor");
		exit (-1);
	}

	//Conecto el socket swap
	conectarConSwap();

	/* Bucle infinito.
	 * Se atiende a si hay más clientes para conectar y a los mensajes enviados
	 * por los clientes ya conectados */
	while (1)
	{

		//Acepto las conexiones y las mando a diferentes threads
		socket_cliente = aceptarConexionCliente(socketServidor);

		//Comienzo el handshake, enviando el tamanio de pagina
		enviarMensajeSocket(socket_cliente,HANDSHAKE_UMC,string_itoa(config->size_pagina));

		//Espero respuesta y creo thread correspondiente
		if(recieve_and_deserialize(package,socket_cliente) > 0){
			logDebug("Mensaje recibido de %d",socket_cliente);
			switch(package->msgCode){
				case -1:
					logDebug("Se desconeccto el cliente %d",socket_cliente);
					break;
				case HANDSHAKE_CPU:
					logDebug("Cliente %d es un CPU",socket_cliente);

					//TODO No estoy liberando esto en ningun momento.
					//Argumentos para el thread de la CPU. Mando una estructura porque es mas facil de modificar en un futuro.
					arg_thread_cpu = malloc(sizeof(t_arg_thread_cpu));
					arg_thread_cpu->socket_cpu=socket_cliente;

					/*TODO Entiendo para este punto deberia estar conectado el nucleo, sino no deberia poder
					 * conectar CPU's. ¿O no es necesario?
					 */
					pthread_create(&thread_tmp,&thread_detached_attr,(void*) handle_cpu,(void*) arg_thread_cpu);
					break;

				case HANDSHAKE_NUCLEO:
					logDebug("Cliente %d es un Nucleo",socket_cliente);
					//TODO No estoy liberando esto en ningun momento.
					arg_thread_nucleo = malloc(sizeof(t_arg_thread_nucleo));
					arg_thread_nucleo->socket_nucleo=socket_cliente;
					pthread_create(&thread_tmp,NULL,(void*) handleNucleo,(void*) arg_thread_nucleo);
					break;

				default:
					logDebug("El cliente %d no se identifico",socket_cliente);
					close(socket_cliente);
			}
		}
	}

	pthread_key_delete(key_pid);
	pthread_attr_destroy(&thread_detached_attr);
}

void comunicarSWAP(int accion){

	//Bloqueo la comunicacion con swap
	pthread_mutex_lock(&comunicacion_swap_mutex);
	pthread_mutex_lock(&socket_swap_mutex);

	int comunicacion_no_exitosa = 1;				//Si la comunicacion no es exitosa intenta de nuevo
	Package* package = malloc(sizeof(Package));	//Paquete para guardar la respuesta

	//Me quedo en un loop hasta lograr comincar el mensaje
	while(comunicacion_no_exitosa){
		if(accion==NUEVO_PROGRAMA_SWAP){

			//esto es con datos de prueba
			int pid = 300;
			int cantidadPaginas = 2;

			char* serialized = serializar_NuevoPrograma(pid,cantidadPaginas);
			int longitud = getLong_NuevoPrograma(cantidadPaginas);

			enviarMensajeSocketConLongitud(socket_swap,accion,serialized,longitud);
			free(serialized);

			//esperar respuesta del Swap si pudo reservar espacio para el nuevo programa
			if(recieve_and_deserialize(package,socket_swap) > 0){

				logDebug("Swap envía [message code]: %d, [Mensaje]: %s", package->msgCode, package->message);

				if(atoi(package->message)){//por ahora solo logeo la respuesta
					logDebug("Swap me avisa que pudo reservar correctamente %d paginas para el Programa PID:%d",cantidadPaginas,pid);
				} else {
					logDebug("Swap me avisa que no pudo reservar %d paginas para el Programa PID:%d",cantidadPaginas,pid);
				}

				/* En un programa posta esto traeria muchos problemas porque si se
				 * corta la comunicacion entre el send y el receive voy a mandar
				 * otro send y reservar el doble de memoria para el mismo programa.
				 *
				 * La forma de solucionarlo seria que el Swap devuelva un codigo
				 * que represente el pedido y el resultado se guarde en el Swap.
				 * Despues la UMC tiene que, con el codigo, pedir el resultado de
				 * la operacion, de esta manera si se corta la comunicacion y luego
				 * se reestablece solo pedis el codigo en vez de volver a pedir memoria.
				 */
				comunicacion_no_exitosa = 0;
			}else
				conectarConSwap();

			destroyPackage(package);
		}
	}

	//Libero la comunicacion con swap
	pthread_mutex_unlock(&socket_swap_mutex);
	pthread_mutex_unlock(&comunicacion_swap_mutex);
}

int conectarConSwap(){

	int socket;		/* descriptor de conexión con el servidor */
	int buffer;		/* buffer de lectura de datos procedentes del servidor */
	int error;		/* error de lectura por el socket */

	pthread_mutex_lock(&socket_swap_mutex);

	/* Se abre una conexión con el servidor */
	socket = abrirConexionInetConServer(config->ip_swap, config->puerto_swap);

	/* Se lee el número de cliente, dato que nos da el servidor.*/
	error = leerSocketClient(socket, (char *)&buffer, sizeof(int));

	/* Si ha habido error de lectura lo indicamos y salimos */
	if (error < 1)
		logDebug("SWAP se encuentra desconectada.");
	else {
		logDebug("Conexion con SWAP satisfactoria.");
		socket_swap=socket;
	}

	pthread_mutex_unlock(&socket_swap_mutex);
	return socket;
}

void handle_cpu(t_arg_thread_cpu* argumentos){
	int sigue = 1,								//Ya se, no es muy original que digamos
		*socket_cpu = &argumentos->socket_cpu,	//Lo guardo en variables para que sea mas comodo de usar
		result;
	Package *package_receive;
	char* contenido_lectura=NULL, *result_serializado=NULL;

	crear_key_pid();

	//Prueba de key_pid borrar cuando haya pid posta
	logDebug("Valor antes de setear un pid %d",obtener_pid());
	setear_pid(rand() % 20);
	logDebug("Valor key seteado %d",obtener_pid());

	while(sigue){
		package_receive = createPackage();
		if(recieve_and_deserialize(package_receive,*socket_cpu) > 0){
			logDebug("CPU envía [message code]: %d, [Mensaje]: %s\n", package_receive->msgCode, package_receive->message);

			switch(package_receive->msgCode){

				case SOLICITAR_BYTES_PAGINA:

					logDebug("Se ha solicitado la lectura de Bytes en pagina.");
					//enviarMensajeSocket(*socket_cpu,SOLICITAR_BYTES_PAGINA,"Bytes leidos");//de prueba

					contenido_lectura=NULL;
					result = leer_pagina(package_receive->message,&contenido_lectura);

					//Si la operacion salio bien result es el tamanio de contenido leido
					if(result > 0){

						//Creo un buffer y serializo el resultado + el contenido leido
						result_serializado=(char*)malloc(sizeof(uint32_t)+result);
						memcpy(result_serializado,&result,sizeof(uint32_t));
						memcpy(result_serializado+sizeof(uint32_t),contenido_lectura,result);

						enviarMensajeSocketConLongitud(*socket_cpu,RESULTADO_OPERACION,result_serializado,sizeof(uint32_t)+result);
						free(result_serializado);

					}else
						enviarMensajeSocketConLongitud(*socket_cpu,RESULTADO_OPERACION,(char*)&result,sizeof(uint32_t));

					break;

				case ALMACENAR_BYTES_PAGINA:

					logDebug("Se ha solicitado la escritura de Bytes en pagina.");
					//enviarMensajeSocket(*socket_cpu,ALMACENAR_BYTES_PAGINA,"Bytes escritos");//de prueba

					result = escribir_pagina(package_receive->message);
					enviarMensajeSocketConLongitud(*socket_cpu,RESULTADO_OPERACION,(char*)&result,sizeof(uint32_t));

					break;

				case SWITCH_PROCESS:

					logDebug("CPU ha solicitado el cambio de Proceso");
					nuevo_pid(package_receive->message);

			}

		} else {
			//Si el cliente cerro la conexion se termino el thread
			sigue=0;
			logInfo("CPU ha cerrado la conexión, cerrando thread");
		}
		destroyPackage(package_receive);
	}
	logInfo("Fin thread CPU pid %d",obtener_pid());
	borrar_key_pid();
}

void crear_key_pid(){
	uint32_t *pid=malloc(sizeof(uint32_t));
	pthread_setspecific(key_pid,(void*) pid);
}

void borrar_key_pid(){
	free(pthread_getspecific(key_pid));
}

void setear_pid(uint32_t pid){
	uint32_t *p_pid;

	p_pid=pthread_getspecific(key_pid);
	*p_pid=pid;
}

uint32_t obtener_pid(){
	uint32_t *pid;
	pid=pthread_getspecific(key_pid);

	if(pid==NULL) return 0;
	else return *pid;
}

void handleNucleo(t_arg_thread_nucleo* args){
	int sigue = 1,
		*socket_nucleo = &args->socket_nucleo,	//Lo guardo en variables para que sea mas comodo de usar
		result;
	Package* package;
	char* contenido_lectura=NULL, *result_serializado=NULL;

	while(sigue){
		package = malloc(sizeof(Package));
		if(recieve_and_deserialize(package,*socket_nucleo) > 0){
			logDebug("Nucleo envía [message code]: %d, [Mensaje]: %s\n", package->msgCode, package->message);

			switch(package->msgCode){

				case INIT_PROGRAM:

					logDebug("Se ha solicitado la inicializacion de un nuevo programa.");

					//comunicarSWAP(NUEVO_PROGRAMA_SWAP);

					result = inicializar_programa(package->message);
					enviarMensajeSocketConLongitud(*socket_nucleo,RESULTADO_OPERACION,(char*)&result,sizeof(uint32_t));

					break;

				case SOLICITAR_BYTES_PAGINA:

					logDebug("Se ha solicitado la lectura de Bytes en pagina.");

					/*enviarMensajeSocket(*socket_nucleo,SOLICITAR_BYTES_PAGINA,"Bytes leidos");//de prueba

					contenido_lectura=NULL;
					result = leer_pagina(package->message,contenido_lectura);

					//Si la operacion salio bien result es el tamanio de contenido leido
					if(result > 0){

						//Creo un buffer y serializo el resultado + el contenido leido
						result_serializado=(char*)malloc(sizeof(uint32_t)+result);
						memcpy(result_serializado,&result,sizeof(uint32_t));
						memcpy(result_serializado+sizeof(uint32_t),contenido_lectura,result);

						enviarMensajeSocketConLongitud(*socket_nucleo,RESULTADO_OPERACION,result_serializado,result);
						free(result_serializado);

					}else
						enviarMensajeSocketConLongitud(*socket_nucleo,RESULTADO_OPERACION,(char*)&result,sizeof(uint32_t));
					*/
					break;

				case ALMACENAR_BYTES_PAGINA:

					logDebug("Se ha solicitado la escritura de Bytes en pagina.");

					/*enviarMensajeSocket(*socket_nucleo,ALMACENAR_BYTES_PAGINA,"Bytes escritos");//de prueba

					result = escribir_pagina(package->message);
					enviarMensajeSocketConLongitud(*socket_nucleo,RESULTADO_OPERACION,(char*)&result,sizeof(uint32_t));
					*/
					break;

				case END_PROGRAM:
					logDebug("Se ha solicitado la finalizacion de un programa.");

					result = finalizar_programa(package->message);
					enviarMensajeSocketConLongitud(*socket_nucleo,RESULTADO_OPERACION,(char*)&result,sizeof(uint32_t));

			}

		} else {
			//Si el cliente cerro la conexion se termino el thread
			sigue=0;
			logInfo("Nucleo ha cerrado la conexión, cerrando thread");
		}
		destroyPackage(package);
	}
	logInfo("Fin thread Nucleo");
}

void inicializarUMC(){

	logDebug("Inicializando la UMC");

	crear_tlb(32); //Todo agregar tamanio de la tlb a la estructura config
	crearMemoriaPrincipal(config->cantidad_paginas, config->size_pagina);
	crearListaDeTablas();
}
