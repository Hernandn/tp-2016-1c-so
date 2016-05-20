/*
 * interfazCPU.h
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#ifndef INTERFAZCPU_H_
#define INTERFAZCPU_H_

#include "configuration.h"

char* serializar_EjecutarInstruccion(uint32_t pid, char* instruccion);
int getLong_EjecutarInstruccion(char* instruccion);

#endif /* INTERFAZCPU_H_ */
