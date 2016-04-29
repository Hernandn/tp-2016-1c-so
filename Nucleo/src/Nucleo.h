/*
 * Nucleo.h
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include <mllibs/sockets/package.h>
#include <commons/log.h>
#include "configuration.h"
#include "PCB.h"

#ifndef NUCLEO_H_
#define NUCLEO_H_

#define MAX_CPUS 10	//cantidad de CPUs que se pueden conectar
#define MAX_CONSOLAS 10	//cantidad de Consolas que se pueden conectar

//codigos de operaciones de la Consola
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define NEW_ANSISOP_PROGRAM 10
#define ANSISOP_PROGRAM 11
#define HANDSHAKE 12
//---------------------
//codigos de operaciones con la CPU
#define CONTINUE_EXECUTION 70
#define ABORT_EXECUTION 71
#define EXEC_NEW_PROCESS 72
//---------------------

//estructura de argumentos para pasarle a un thread
typedef struct arg_struct {
    int cpuSockets[MAX_CPUS];
    int consolaSockets[MAX_CONSOLAS];
    int socketServerCPU;
    int socketServerConsola;
    int socketServerUMC;
    int socketServerThreads;
    t_list* listaCPUs;
    Configuration* config;
    Estados* estados;
    t_log* logger;
} arg_struct;

//contiene el estado de un CPU conectado al Nucleo
typedef struct CPU {
    int cpuFD;	//file descriptor del socket del cpu
    int libre;	// 0:ocupado / 1:libre
} CPU;

//prototipos de funciones
void handleClients(Configuration* config, t_log* logger);
void handleConsolas();
void handleCPUs();
int elegirRandomCPU(int cpuSockets[]);
void comunicarCPU(int cpuSockets[]);
void enviarMensajeCPU(Package* package,int socket);
Configuration* configurar(t_log* logger);
void imprimirArraySockets(int sockets[], int len);
void inicializarArraySockets(arg_struct* args);
int conectarConUMC(Configuration* config, t_log* logger);
void nuevoCPU(t_list* listaCPUs, int socketCPU);
void destroyCPU(CPU* self);

#endif /* NUCLEO_H_ */
