/*
 * interfaz.h
 *
 *  Created on: 26/5/2016
 *      Author: utnso
 */

#ifndef NUCLEOCPU_INTERFAZ_H_
#define NUCLEOCPU_INTERFAZ_H_

#include <stdio.h>
#include <stdlib.h>
#include "../sockets/server.h"
#include "../sockets/client.h"
#include "../sockets/package.h"
#include "../log/logger.h"
#include <stdint.h>
#include <parser/metadata_program.h>

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
#define EXEC_IO_OPERATION 79
#define CPU_LIBRE 90
//-------------------------------

typedef struct PCB {
	uint32_t processID;		//identificador unico del proceso
	int consolaFD;		//file descriptor del socket de la consola que inicio el programa
	uint32_t programCounter;	//contador de programa
	uint32_t pagesQty;		//cantidad de paginas de codigo ANSISOP
	int executedQuantums;	//cantidad de quantums ya ejecutados
	t_metadata_program* codeIndex;		//indice de codigo
	int* stackIndex;		//indice del stack (todavia no implementado)
	bool consolaActiva;	//indica si la consola esta activa o si cerro la conexion
	char* programa;	//codigo del programa
} PCB;

//wrapper que se crea con cada solicitud de I/O de un proceso
//va a la cola de bloqueados correspondiente al dispositivo (io_id)
typedef struct solicitud_io {
    PCB* pcb;
    char* io_id;
	int cant_operaciones;
} solicitud_io;

//serializacion PCB

void destroyPCB(PCB* self);
char* serializarPCB(PCB* pcb);
uint32_t getLong_PCB(PCB* pcb);
char* serializar_metadata_program(t_metadata_program* metadata);
uint32_t getLong_metadata_program(t_metadata_program* metadata);
PCB* deserializar_PCB(char* serialized);
t_metadata_program* deserializar_metadata_program(char* serialized);
char* serializar_ejecutarOperacionIO(PCB* pcb, char* io_id, uint32_t cant_operaciones);
uint32_t getLong_ejecutarOperacionIO(PCB* pcb, char* io_id, uint32_t cant_operaciones);
solicitud_io* deserializar_ejecutarOperacionIO(char* serialized);



//funciones interfaz CPU a Nucleo

void informarNucleoFinPrograma(int socketNucleo, PCB* pcb);
void informarNucleoQuantumFinished(int socketNucleo, PCB* pcb);
void informarNucleoContextSwitchFinished(int socketNucleo, PCB* pcb);
void informarNucleoCPUlibre(int socketNucleo);
void informarNucleoEjecutarOperacionIO(int socketNucleo, PCB* pcb, char* io_id, uint32_t cant_operaciones);

//funciones interfaz Nucleo a CPU

void ejecutarNuevoProcesoCPU(int socketCPU, PCB* pcb);
void continuarEjecucionProcesoCPU(int socketCPU);

#endif /* NUCLEOCPU_INTERFAZ_H_ */
