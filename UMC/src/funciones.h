/*
 * funciones.h
 *
 *  Created on: 8/5/2016
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include <mllibs/sockets/server.h>
#include <mllibs/sockets/client.h>
#include <mllibs/sockets/package.h>
#include <pthread.h>
#include <stdlib.h>
#include <commons/bitarray.h>
#include "interfaz.h"
#include "configuration.h"

#define MAX_CLIENTES 10 //cantidad maxima de conexiones por socket (CPUs)

//codigos de operaciones de la UMC
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define INIT_PROGRAM 1
#define SOLICITAR_BYTES_PAGINA 2
#define ALMACENAR_BYTES_PAGINA 3
#define END_PROGRAM 4
#define HANDSHAKE_UMC 5
#define SWITCH_PROCESS 6
//---------------------
//operaciones SWAP
#define SOLICITAR_PAGINA_SWAP 20
#define ALMACENAR_PAGINA_SWAP 21
#define NUEVO_PROGRAMA_SWAP 22
#define ELIMINAR_PROGRAMA_SWAP 23
//---------------------
//codigos de operacion con el Nucleo
#define HANDSHAKE_NUCLEO 12
//---------------------
//codigos de operacion con los CPU's
#define HANDSHAKE_CPU 5

typedef struct arg_thread_cpu {
	int socket_cpu;
} t_arg_thread_cpu;

typedef struct arg_thread_nucleo {
	int socket_nucleo;
	Configuration* config;
} t_arg_thread_nucleo;

typedef char* pagina;
typedef void* memoria; //TODO No me convence mucho el nombre pero es lo primero que se me ocurrio, sean libres de modificarlo

typedef struct memoria_principal{
	memoria memoria;
	t_bitarray* bitmap;
} t_memoria_principal;

typedef struct fila_tabla{
	uint16_t numero_pagina;
	uint16_t numero_marco;
	char activo;
} t_fila_tabla;

typedef struct tabla{
	t_fila_tabla* filas;
	uint32_t pid;
} t_tabla;

pthread_mutex_t retardo_mutex;

void handleClients();
void comunicarSWAP(int);
int conectarConSwap();
void inicializarUMC();
void handle_cpu(t_arg_thread_cpu*);
void handleNucleo(t_arg_thread_nucleo*);
void retardo (int segundos);
void handleComandos();
void intepretarComando(char*);
void error_comando(char*);
void flush_memory();
void flush_tlb();
void retardo (int);
void dump ();
int parsear_comando(char*, char **);
void fin_programa();
void print_retardo();
void limpiar_pantalla();
t_memoria_principal crearMemoriaPrincipal(int, int);
void crear_tabla_de_paginas(uint32_t,uint32_t);
void crear_tlb(uint32_t,uint32_t);
void insertar_tabla(t_tabla*,t_tabla**,int,int);

#endif /* FUNCIONES_H_ */
