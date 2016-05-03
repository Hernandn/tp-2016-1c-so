/*
 * CPU.h
 *
 *  Created on: 19/4/2016
 *      Author: utnso
 */

#include "configuration.h"

#ifndef CPU_H_
#define CPU_H_

//codigos de operaciones de la UMC
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define INIT_PROGRAM 1
#define SOLICITAR_BYTES_PAGINA 2
#define ALMACENAR_BYTES_PAGINA 3
#define END_PROGRAM 4
#define HANDSHAKE 5
#define SWITCH_PROCESS 6
//---------------------
//codigos de operaciones entre CPU/Nucleo
#define CONTINUE_EXECUTION 70
#define ABORT_EXECUTION 71
#define EXEC_NEW_PROCESS 72
#define EXECUTION_FINISHED 73
#define QUANTUM_SLEEP_CPU 74
#define QUANTUM_FINISHED 75
#define CPU_LIBRE 90
//----------------------
#define NEW_ANSISOP_PROGRAM 10

typedef struct arg_struct {
	Configuration* config;
	int socketUMC;
	int socketNucleo;
	int processID;//processID del proceso que actualmente se esta ejecutando (despues va a ser el PCB directamente)
} arg_struct;

//prototipos de funciones
void conectarConUMC();
void conectarConNucleo();
Configuration* configurar();
void comunicarUMC(int socketUMC, int accion);
void enviarMensaje(int socket, int accion, char* message);
void analizarMensaje(Package package, arg_struct *args);
void ejecutarProceso(arg_struct *args, Package package);
void quantumSleep(arg_struct *args, int milisegundos);
void abortarProceso(arg_struct *args);


#endif /* CPU_H_ */
