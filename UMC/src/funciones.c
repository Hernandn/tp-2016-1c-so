/*
 * funciones.c
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include "funciones.h"
#include "interfazSwap.h"

//----------------------------------PRIVADO---------------------------------------

static t_memoria_principal memoria_principal;
static t_tabla** tablas_de_paginas=NULL;
static t_fila_tlb* tlb=NULL;
static int maximo_tablas_de_paginas=0;
static int cant_tablas_de_paginas=0;

static int socket_swap = -1;

static pthread_mutex_t
	comunicacion_swap_mutex,
	socket_swap_mutex,
	continua_mutex;

static int continua=1;

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

	crear_key_pid();

	//Prueba de key_pid borrar cuando haya pid posta
	logDebug("Valor antes de setear un pid %d",obtener_pid());
	setear_pid(rand() % 20);
	logDebug("Valor key seteado %d",obtener_pid());

	while(sigue){
		 package = malloc(sizeof(Package));
		if(recieve_and_deserialize(package,*socket_cpu) > 0){
			logDebug("CPU envía [message code]: %d, [Mensaje]: %s\n", package->msgCode, package->message);
			if(package->msgCode==INIT_PROGRAM){
				logDebug("Se ha solicitado la inicializacion de un nuevo programa.");
				//inicializar_programa(package->message);
				comunicarSWAP(NUEVO_PROGRAMA_SWAP);
			} else if(package->msgCode==SOLICITAR_BYTES_PAGINA){
				logDebug("Se ha solicitado la lectura de Bytes en pagina.");
				//leer_pagina(package->message);
				enviarMensajeSocket(*socket_cpu,SOLICITAR_BYTES_PAGINA,"Bytes leidos");//de prueba
			} else if(package->msgCode==ALMACENAR_BYTES_PAGINA){
				logDebug("Se ha solicitado la escritura de Bytes en pagina.");
				//escribir_pagina(package->message);
				enviarMensajeSocket(*socket_cpu,ALMACENAR_BYTES_PAGINA,"Bytes escritos");//de prueba
			}
		} else {
			//Si el cliente cerro la conexion se termino el thread
			sigue=0;
			logInfo("CPU ha cerrado la conexión, cerrando thread");
		}
		//Todo el destroyPackage tira violacion de segmento
		//destroyPackage(package);
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
		*socket_nucleo = &args->socket_nucleo;	//Lo guardo en variables para que sea mas comodo de usar
	Package* package;

	while(sigue){
		package = malloc(sizeof(Package));
		if(recieve_and_deserialize(package,*socket_nucleo) > 0){
			logDebug("Nucleo envía [message code]: %d, [Mensaje]: %s\n", package->msgCode, package->message);
			if(package->msgCode==INIT_PROGRAM){
				logDebug("Se ha solicitado la inicializacion de un nuevo programa.");
				//inicializar_programa(package->message);
				comunicarSWAP(NUEVO_PROGRAMA_SWAP);
			} else if(package->msgCode==SOLICITAR_BYTES_PAGINA){
				logDebug("Se ha solicitado la lectura de Bytes en pagina.");
				//leer_pagina(package->message);
				enviarMensajeSocket(*socket_nucleo,SOLICITAR_BYTES_PAGINA,"Bytes leidos");//de prueba
			} else if(package->msgCode==ALMACENAR_BYTES_PAGINA){
				logDebug("Se ha solicitado la escritura de Bytes en pagina.");
				//escribir_pagina(package->message);
				enviarMensajeSocket(*socket_nucleo,ALMACENAR_BYTES_PAGINA,"Bytes escritos");//de prueba
			} else if(package->msgCode==END_PROGRAM){
				logDebug("Se ha solicitado la finalizacion de un programa.");
				//finalizar_programa(package->message);
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

	tlb=crear_tlb(32);	//Todo agregar el tamanio de la tlb a la estructura config
	memoria_principal=crearMemoriaPrincipal(config->cantidad_paginas, config->size_pagina);
}

t_memoria_principal crearMemoriaPrincipal(int cantidad_paginas, int size_pagina){

	t_memoria_principal memoria;
	char* bits = malloc(sizeof(char)*cantidad_paginas);
	int i;

	logDebug("Creando memoria principal de tamanio %d\n", cantidad_paginas*size_pagina);

	for(i=0;i<cantidad_paginas;i++){
		bits[i]=0;
	}

	memoria.memoria = malloc(cantidad_paginas*size_pagina);
	memoria.bitmap = bitarray_create(bits,cantidad_paginas);

	return memoria;
}

void crear_tabla_de_paginas(uint32_t pid, uint32_t cant_paginas){

	t_tabla *nueva_tabla = malloc(sizeof(t_tabla));
	int i;

	nueva_tabla->pid = pid;
	nueva_tabla->tamanio = cant_paginas;
	nueva_tabla->filas = malloc(sizeof(t_fila_tabla)*cant_paginas);

	for(i=0; i<cant_paginas; i++){
		nueva_tabla->filas[i].modificacion = 0;
	}

	insertar_tabla(nueva_tabla,tablas_de_paginas,maximo_tablas_de_paginas,cant_tablas_de_paginas);

}

void eliminar_tabla_de_paginas(uint32_t pid){

	t_tabla *tabla;

	logDebug("Eliminando tabla con pid %d", pid);

	tabla=obtener_tabla_de_paginas(pid);

	eliminar_tabla(tabla, tablas_de_paginas, cant_tablas_de_paginas);

	free(tabla->filas);
	free(tabla);
}

t_tabla* obtener_tabla_de_paginas(uint32_t pid){

	t_tabla *tabla=*tablas_de_paginas;
	int i=0;

	while(tabla->pid != pid && i <= maximo_tablas_de_paginas){
		tabla++;
		i++;
	}

	if(tabla->pid==pid) return tabla;

	return NULL;
}

t_fila_tlb* crear_tlb(uint32_t cant_paginas){

	t_fila_tlb *nueva_tabla = malloc(sizeof(t_fila_tlb)*cant_paginas);
	int i;

	logDebug("Nueva TLB creada con %d paginas",cant_paginas);

	//Inicializo los pids en 0
	for(i=0; i<cant_paginas; i++){
		nueva_tabla[i].pid=0;
	}

	return nueva_tabla;
}

void insertar_tabla(t_tabla *nueva_tabla, t_tabla **lista_tablas, int cant_maximo_tablas, int cant_tablas){

	int i;

	if(cant_tablas >= cant_maximo_tablas){
		lista_tablas=realloc((void*)lista_tablas,(sizeof(t_tabla*)*5)+cant_maximo_tablas);
		cant_maximo_tablas+=5;

		//Inicializo las tablas en null
		for(i=cant_tablas+1; i<cant_maximo_tablas; i++){
			lista_tablas[i]=NULL;
		}
	}

	//Sumo una tabla mas a la cantidad de tablas
	cant_tablas += 1;

	//Agrego la tabla a la lista
	for(i=0; i<cant_maximo_tablas; i++){
		if(lista_tablas[i]==NULL){
			lista_tablas[i] = nueva_tabla;
		}
	}

}

void eliminar_tabla(t_tabla *tabla, t_tabla **lista_tablas, int cant_tablas){

	int i=0;

	//Busco el indice de la tabla
	while(lista_tablas[i]==tabla && i<=cant_tablas){
		i++;
	}

	//Pongo el puntero en NULL
	lista_tablas[i]=NULL;
}

uint32_t obtener_dir_fisica(uint32_t dir_logica){
	//Todo buscar la dir en la tlb
	return 0;
}

char* obtener_contenido_memoria(uint32_t dir_fisica, uint32_t offset, uint32_t tamanio){
	//Todo busar en memoria el contenido
	return NULL;
}

int escribir_contenido_memoria(uint32_t dir_fisica, uint32_t offset, uint32_t tamanio, char* contenido){
	//Todo escribir contenido en memoria
	return 0;
}

void handleComandos(){

	char * comando;
	size_t size_buff=0;

	//Mutex para manejar el flag de ejecucion
	pthread_mutex_init(&continua_mutex,NULL);

	pthread_mutex_lock(&continua_mutex);
	while(continua){

		comando = NULL;
		printf("ml-umc>");
		getline(&comando,&size_buff,stdin);
		intepretarComando(comando);

		free(comando);
	}
	pthread_mutex_unlock(&continua_mutex);
}

void intepretarComando(char* comando){

	char** comando_parseado = malloc(sizeof(char*)*2);
	int cantidad;

	//TODO no estoy liberando comando_parseado en ningun momento
	cantidad=parsear_comando(comando, comando_parseado);

	if(!strcmp(*comando_parseado,"dump")) dump();
	else if(!strcmp(*comando_parseado,"flush") && (cantidad == 2))

			if(!strcmp(*(comando_parseado+1),"tlb")) flush_tlb();
			else if (!strcmp(*(comando_parseado+1),"memory")) flush_memory();
				 else error_comando(comando);

		 else if(!strcmp(*comando_parseado,"retardo"))

			if(cantidad == 2) retardo(atoi(*(comando_parseado + 1)));
			else print_retardo();

			else if(!strcmp(*comando_parseado,"clear")) limpiar_pantalla();
				 else if(!strcmp(*comando_parseado,"exit")) fin_programa();
				 	  else error_comando(comando);
}

int parsear_comando(char * comando, char ** comando_parseado){

	int i=0;				//Indice para recorrer el string
	int contador = 0;		//Contador de palabras parseadas
	char * tmp = comando;	//Puntero al comienzo de la proxima palabra a parsear
	int letras = 0;			//Conador de letras de la palara a parsear

	while(*(comando + i) != '\0'){
		if (*(comando + i) == 32){
			if(letras != 0){
				*(comando_parseado + contador) = malloc(sizeof(char)*letras+1);
				strncpy(*(comando_parseado + contador),tmp,letras);
				*(*(comando_parseado + contador)+letras) = '\0';
				contador ++;
			}
			tmp=tmp+i+1;
			letras = 0;
		}else {
			letras++;
		}

		i++;
	}

	*(comando_parseado + contador)= malloc(sizeof(char)*letras+1);
	strncpy(*(comando_parseado + contador),tmp,letras);

	//getline tambien guarda el \n y hay que eliminarlo para poder comparar despues
	*(*(comando_parseado + contador)+letras-1) = '\0';

	return contador+1;
}

void dump ()
{
	printf("Este es un reporte de prueba del estado dump\n");

}

void retardo (int segundos)
{
	pthread_mutex_lock(&retardo_mutex);
	config->retraso = segundos;
	pthread_mutex_unlock(&retardo_mutex);
}

void print_retardo(){
	pthread_mutex_lock(&retardo_mutex);
	printf("Retardo actual: %d\n",config->retraso);
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

void error_comando(char* comando)
{
	printf("Comando inexistente %s", comando);
}

void limpiar_pantalla(){

	system("clear");

}

void fin_programa(){

	/*Se cambia la variable "continua" a 0 para que
	 * todos los threads sepa que se ejecuto el comando exit
	 */

	continua = 0;

}
