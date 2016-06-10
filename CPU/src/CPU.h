/*
 * CPU.h
 *
 *  Created on: 19/4/2016
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include "configuration.h"
#include <mllibs/nucleoCpu/interfaz.h>

//codigos de operaciones de la UMC
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define INIT_PROGRAM 1
#define SOLICITAR_BYTES_PAGINA 2
#define ALMACENAR_BYTES_PAGINA 3
#define END_PROGRAM 4
#define HANDSHAKE_CPU 5
#define SWITCH_PROCESS 6
//---------------------
//codigos de operaciones entre CPU/Nucleo
#define CONTINUE_EXECUTION 70
#define ABORT_EXECUTION 71
#define EXEC_NEW_PROCESS 72
#define EXECUTION_FINISHED 73
#define QUANTUM_SLEEP_CPU 74
#define QUANTUM_FINISHED 75
#define PROGRAM_FINISHED 76
#define CONTEXT_SWITCH 77
#define CONTEXT_SWITCH_FINISHED 78
#define CPU_LIBRE 90
//----------------------
//codigos de operaciones con UMC
#define HANDSHAKE_UMC 5
//----------------------
#define NEW_ANSISOP_PROGRAM 10

typedef struct arg_struct {
	Configuration* config;
} arg_struct;


int socketUMC;
int socketNucleo;

int proceso_fue_bloqueado;
int programa_finalizado;
int end_signal_received;
int end_cpu;

int size_pagina;

PCB* pcbActual;//pcb del proceso que se esta ejecutando actualmente en el CPU

//prototipos de funciones
void conectarConUMC(void*);
void iniciarEjecucionCPU(void*);
Configuration* configurar();
void comunicarUMC(int socketUMC, int accion);
void enviarMensaje(int socket, int accion, char* message);
void analizarMensaje(Package* package, arg_struct *args);
void contextSwitch();
void cargarContextoPCB(Package* package);
void ejecutarProceso(arg_struct *args);
void quantumSleep(arg_struct *args, int milisegundos);
bool programaFinalizado();
void abortarProceso(arg_struct *args);
void ejecutarInstruccion();
void analizarRespuestaUMC();
int getSocketUMC();
char* getInstruccion(char* codigo, int offset, int length);
char* getSiguienteInstruccion();
char* getInstruccionFromUMC(int offset, int length);
char* pedirCodigoUMC(uint32_t pagina, uint32_t offset, uint32_t size);
void ejecutarOperacionIO(char* io_id, uint32_t cant_operaciones);
void finalizarPrograma();


#endif /* CPU_H_ */
