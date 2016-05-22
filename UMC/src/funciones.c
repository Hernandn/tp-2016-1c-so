/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include "funciones.h"
#include "interfazSwap.h"

memoria memoria_principal;
tableRow* tabla;
static int socket_swap = -1;

static pthread_mutex_t
	comunicacion_swap_mutex,
	socket_swap_mutex;

void handleClients(){

	int socketServidor,							//Descriptor del socket servidor
		socket_cliente;							//Se usa para recivir al conexion y se envia al thread que la va a manejar
	pthread_t thread_tmp;						//Variable Temporal para crear threads;
	t_arg_thread_cpu* arg_thread_cpu;			//Arguemntos para el thread del nuevo cpu
	t_arg_thread_nucleo* arg_thread_nucleo;		//Argumentos para el thread del nuevo Nucleo
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
					logDebug("Cliente %d es un CPU"),socket_cliente;

					//TODO No estoy liberando esto en ningun momento.
					//Argumentos para el thread de la CPU. Mando una estructura porque es mas facil de modificar en un futuro.
					arg_thread_cpu = malloc(sizeof(t_arg_thread_cpu));
					arg_thread_cpu->socket_cpu=socket_cliente;

					/*TODO Entiendo para este punto deberia estar conectado el nucleo, sino no deberia poder
					 * conectar CPU's. ¿O no es necesario?
					 */
					pthread_create(&thread_tmp,NULL,(void*) handle_cpu,(void*) arg_thread_cpu);
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
}

void comunicarSWAP(int socket_swap, int accion){

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
			int longitud = getLong_NuevoPrograma();

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
	int sigue = 1,	//Ya se, no es muy original que digamos
		*socket_cpu = &argumentos->socket_cpu;	//Lo guardo en variables para que sea mas comodo de usar
	Package* package;

	while(sigue){
		 package = malloc(sizeof(Package));
		if(recieve_and_deserialize(package,*socket_cpu) > 0){
			logDebug("CPU envía [message code]: %d, [Mensaje]: %s\n", package->msgCode, package->message);
			if(package->msgCode==INIT_PROGRAM){
				logDebug("Se ha solicitado la inicializacion de un nuevo programa.");
				comunicarSWAP(socket_swap,NUEVO_PROGRAMA_SWAP);
			} else if(package->msgCode==SOLICITAR_BYTES_PAGINA){
				logDebug("Se ha solicitado la lectura de Bytes en pagina.");
				enviarMensajeSocket(*socket_cpu,SOLICITAR_BYTES_PAGINA,"Bytes leidos");//de prueba
			} else if(package->msgCode==ALMACENAR_BYTES_PAGINA){
				logDebug("Se ha solicitado la escritura de Bytes en pagina.");
				enviarMensajeSocket(*socket_cpu,ALMACENAR_BYTES_PAGINA,"Bytes escritos");//de prueba
			}
		} else {
			//Si el cliente cerro la conexion se termino el thread
			sigue=0;
			logInfo("Cliente ha cerrado la conexión, cerrando thread");
		}
		destroyPackage(package);
	}
	logInfo("Fin thread CPU");
}

void handleNucleo(t_arg_thread_nucleo* args){
	int sigue = 1,
		*socket_nucleo = &args->socket_nucleo;	//Lo guardo en variables para que sea mas comodo de usar
	Package* package;

	while(sigue){
		package = malloc(sizeof(Package));
		if(recieve_and_deserialize(package,*socket_nucleo) > 0){
			logDebug("Nucleo envía [message code]: %d, [Mensaje]: %s\n", package->msgCode, package->message);
			if(package->msgCode==INIT_PROGRAM){
				logDebug("Se ha solicitado la inicializacion de un nuevo programa.");
				comunicarSWAP(socket_swap,NUEVO_PROGRAMA_SWAP);
			} else if(package->msgCode==SOLICITAR_BYTES_PAGINA){
				logDebug("Se ha solicitado la lectura de Bytes en pagina.");
				enviarMensajeSocket(*socket_nucleo,SOLICITAR_BYTES_PAGINA,"Bytes leidos");//de prueba
			} else if(package->msgCode==ALMACENAR_BYTES_PAGINA){
				logDebug("Se ha solicitado la escritura de Bytes en pagina.");
				enviarMensajeSocket(*socket_nucleo,ALMACENAR_BYTES_PAGINA,"Bytes escritos");//de prueba
			} else if(package->msgCode==END_PROGRAM){
				logDebug("Se ha solicitado la finalizacion de un programa.");
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

	tabla = crearTablaDePaginas(config->cantidad_paginas);
	memoria_principal=malloc(config->cantidad_paginas*config->size_pagina);
	logDebug("Creando memoria principal de tamanio %d\n", config->cantidad_paginas*config->size_pagina);
}

tableRow* crearTablaDePaginas(int cantidadFrames){
	tableRow* table = malloc(sizeof(tableRow)*cantidadFrames);
	logDebug("Creando tabla de paginas con %d paginas\n",cantidadFrames);
	return table;
}

void handleComandos()
{
	char comando[10];
	printf("pone el comando \n");
	fgets(comando, sizeof(char)*10,stdin);
	intepretarComando(comando);
	printf("termine handle comando\n");
}

void intepretarComando(char* comando)
{
	char** comando_parseado = malloc (sizeof(char*)*2);
	int cantidad;

	cantidad=parsear_comando(comando, comando_parseado);

	if (!(strcmp(*comando_parseado,"dump")))
	{
		printf("entro if dump\n");
		dump();
	}
	else if (!(strcmp(*comando_parseado,"flush")) && (cantidad ==2))
	{
		if (!(strcmp(*(comando_parseado+1),"tlb")))
				{
					printf("entro if  tlb\n");
					flush_tlb();

				}
		else if (!(strcmp(*(comando_parseado+1),"memory")))
		{
			printf("entro if memory\n");
			flush_memory();

		}
		else
		{
			error_comando(comando);
		}

	}
	else if (!(strcmp(*comando_parseado,"retardo")) && (cantidad ==2))
	{
		printf("entro if retardo\n");
		retardo(atoi(*(comando_parseado + 1)));

	}
	else
	{
		printf("entro else de comando\n");
		error_comando(comando);
	}
	free(comando_parseado);
}

int parsear_comando(char * comando, char ** comando_parseado)
{
	int i=0;
	int contador = 0;
	char * tmp = comando;
	int palabra = 1;

	while(*(comando + i) != '\0')
	{
		if (*(comando + i)  == 32)
		{
			*(comando_parseado + contador) = malloc(sizeof(char)*palabra);
			fflush(stdin);
			strncpy(*(comando_parseado + contador),tmp,palabra);
			fflush(stdin);
			*(*(comando_parseado + contador)+palabra-1) = '\0';
			contador ++;
			tmp=tmp+i+1;
			palabra = 1;
		}
		i++;
		palabra++;

	}
	*(comando_parseado + contador)= malloc(sizeof(char)*palabra);
	strncpy(*(comando_parseado + contador),tmp,palabra);
	*(*(comando_parseado + contador)+palabra-1) = '\0';

	printf("comando recibido %s \n", comando);
	printf("comando parseado %s \n", *comando_parseado);
	free(tmp);

	return contador+1;
}
void dump ()
{
	printf("Este es un reporte de prueba del estado\n");

}

void retardo (int segundos)
{

	pthread_mutex_lock(&retardo_mutex);
	// aca tengo que modificar el retardo que da en el config
	printf("hola soy el retardo y mis segundos son %d \n",segundos);
	sleep (segundos);
	pthread_mutex_unlock(&retardo_mutex);
}

void flush_tlb()
{

	printf("hola soy el flush tlb\n");
}

void flush_memory()
{
	printf("hola soy el flush memory\n");
}

void error_comando(char * comando)
{
	printf("comando inexistente\n");
	fflush(stdin);
}
