/*
 * interfazCPU.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "interfazCPU.h"

char* serializar_EjecutarInstruccion(uint32_t pid, char* instruccion){
	//mensaje: pid + cantPags + paginas
	char *serializedPackage = malloc(sizeof(uint32_t)+strlen(instruccion)+1);

	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &pid, size_to_send);
	offset += size_to_send;

	size_to_send = strlen(instruccion)+1;
	memcpy(serializedPackage + offset, instruccion, size_to_send);

	return serializedPackage;
}

int getLong_EjecutarInstruccion(char* instruccion){
	return sizeof(uint32_t)+sizeof(char)*strlen(instruccion)+1;
}
