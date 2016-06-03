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
#include "../stack/stack.h"
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

typedef struct dir_memoria {
	uint32_t pagina;
	uint32_t offset;
	uint32_t size;
} dir_memoria;

typedef struct contexto {
	t_dictionary* argumentos;//coleccion de pares ("identificador_variable",dir_memoria)
	t_dictionary* variables;//coleccion de pares ("identificador_variable",dir_memoria)
    t_puntero_instruccion retPos;
	dir_memoria retVar;
} contexto;

typedef struct PCB {
	uint32_t processID;		//identificador unico del proceso
	int consolaFD;		//file descriptor del socket de la consola que inicio el programa
	uint32_t programCounter;	//contador de programa
	uint32_t stackFirstPage;		//numero de pagina de inicio del stack en la UMC
	uint32_t stackOffset;	//offset actual donde agregar variables en el stack
	int executedQuantums;	//cantidad de quantums ya ejecutados
	t_metadata_program* codeIndex;		//indice de codigo
	t_stack* stackIndex;		//pila con elementos "contexto"
	bool consolaActiva;	//indica si la consola esta activa o si cerro la conexion
	char* programa;	//codigo del programa TODO: borrar
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
void serializarDato(char* buffer, void* dato, int size_to_send, int* offset);
void deserializarDato(void* dato, char* buffer, int size, int* offset);
char* serializarPCB(PCB* pcb);
uint32_t getLong_PCB(PCB* pcb);
char* serializar_metadata_program(t_metadata_program* metadata);
uint32_t getLong_metadata_program(t_metadata_program* metadata);
PCB* deserializar_PCB(char* serialized);
t_metadata_program* deserializar_metadata_program(char* serialized);
char* serializar_ejecutarOperacionIO(PCB* pcb, char* io_id, uint32_t cant_operaciones);
uint32_t getLong_ejecutarOperacionIO(PCB* pcb, char* io_id, uint32_t cant_operaciones);
solicitud_io* deserializar_ejecutarOperacionIO(char* serialized);
char* serializar_contexto(contexto* contexto);
uint32_t getLong_contexto(contexto* contexto);
contexto* deserializar_contexto(char* serialized);
char* serializar_dictionary(t_dictionary* dictionary);
uint32_t getLong_dictionary(t_dictionary* dictionary);
t_dictionary* deserializar_dictionary(char* serialized, uint32_t size);
char* serializar_stack(t_stack* stack);
uint32_t getLong_stack(t_stack* stack);
t_stack* deserializar_stack(char* serialized);
void crearNuevoContexto(PCB* pcb);
void destroy_contexto(contexto* contexto);
void destroy_dir_memoria(dir_memoria* dir);


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
