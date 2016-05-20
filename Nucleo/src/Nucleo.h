/*
 * Nucleo.h
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include "configuration.h"
#include "PCB.h"
#include "planificador.h"


#ifndef NUCLEO_H_
#define NUCLEO_H_

#define MAX_CPUS 10	//cantidad de CPUs que se pueden conectar
#define MAX_CONSOLAS 10	//cantidad de Consolas que se pueden conectar

//codigos de operaciones de la Consola
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define NEW_ANSISOP_PROGRAM 10
#define ANSISOP_PROGRAM 11
#define HANDSHAKE_NUCLEO 12
//---------------------
//codigos de operacion con la UMC
#define HANDSHAKE_UMC 5
//---------------------
//codigos de operaciones entre CPU/Nucleo
#define CONTINUE_EXECUTION 70
#define ABORT_EXECUTION 71
#define EXEC_NEW_PROCESS 72
#define EXECUTION_FINISHED 73
#define QUANTUM_SLEEP_CPU 74
#define QUANTUM_FINISHED 75
#define CPU_LIBRE 90
//---------------------

//estructura de argumentos para pasarle a un thread
typedef struct arg_struct {
    int cpuSockets[MAX_CPUS];
    int consolaSockets[MAX_CONSOLAS];
    int socketServerCPU;
    int socketServerConsola;
    int socketServerUMC;
    int socketServerPlanificador;
    int socketClientPlanificador;
    t_list* listaCPUs;
    Configuration* config;
    Estados* estados;
} arg_struct;

//contiene el estado de un CPU conectado al Nucleo
typedef struct CPU {
    int cpuFD;	//file descriptor del socket del cpu
    int libre;	// 0:ocupado / 1:libre
} CPU;

//prototipos de funciones
void handleClients(Configuration* config);
void handleConsolas();
void handleCPUs();
int elegirRandomCPU(int cpuSockets[]);
void comunicarCPU(int cpuSockets[]);
void enviarMensaje(int socket, int accion, char* message);
Configuration* configurar();
void imprimirArraySockets(int sockets[], int len);
void inicializarArraySockets(arg_struct* args);
int conectarConUMC(Configuration* config);
void nuevoCPU(t_list* listaCPUs, int socketCPU, int socketPlanificador);
void destroyCPU(CPU* self);
void liberarCPU(CPU* cpu, int socketPlanificador);
void liberarCPUporSocketFD(int socketCPU, arg_struct *args);
CPU* buscarCPUporSocketFD(int socketCPU, t_list* listaCPUs);
void eliminarCPU(t_list* listaCPUs,int socketCPU);
int conectarConPlanificador(char* ip, int puerto);
void analizarMensajeCPU(int socketCPU , Package* package, arg_struct *args);

#endif /* NUCLEO_H_ */
