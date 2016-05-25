/*
 * interfazCPU.h
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#ifndef INTERFAZCPU_H_
#define INTERFAZCPU_H_

#include "configuration.h"
#include "Nucleo.h"
#include <parser/metadata_program.h>

char* serializar_EjecutarInstruccion(uint32_t pid, char* instruccion);
uint32_t getLong_EjecutarInstruccion(char* instruccion);

void ejecutarNuevoProcesoCPU(int socketCPU, PCB* pcb);
void continuarEjecucionProcesoCPU(int socketCPU);

char* serializarPCB(PCB* pcb);
uint32_t getLong_PCB(PCB* pcb);
char* serializar_metadata_program(t_metadata_program* metadata);
uint32_t getLong_metadata_program(t_metadata_program* metadata);
PCB* deserializar_PCB(char* serialized);
t_metadata_program* deserializar_metadata_program(char* serialized);

#endif /* INTERFAZCPU_H_ */
