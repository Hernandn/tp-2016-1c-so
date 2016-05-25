/*
 * interfazNucleo.h
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#ifndef INTERFAZNUCLEO_H_
#define INTERFAZNUCLEO_H_

#include "configuration.h"
#include "CPU.h"
#include <parser/metadata_program.h>

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

uint32_t getProcessID_ejecutarInstruccion(char* str);
char* getInstruccion_ejecutarInstruccion(char* str);

//PCB

char* serializarPCB(PCB* pcb);
uint32_t getLong_PCB(PCB* pcb);
char* serializar_metadata_program(t_metadata_program* metadata);
uint32_t getLong_metadata_program(t_metadata_program* metadata);
PCB* deserializar_PCB(char* serialized);
t_metadata_program* deserializar_metadata_program(char* serialized);

void informarNucleoFinPrograma(int socketNucleo, PCB* pcb);
void informarNucleoQuantumFinished(int socketNucleo, PCB* pcb);
void informarNucleoContextSwitchFinished(int socketNucleo, PCB* pcb);
void informarNucleoCPUlibre(int socketNucleo);

#endif /* INTERFAZNUCLEO_H_ */
