/*
 * interfazNucleo.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "interfazNucleo.h"

uint32_t getProcessID_ejecutarInstruccion(char* str){
	uint32_t pid;
	memcpy(&pid,str,sizeof(uint32_t));
	return pid;
}

char* getInstruccion_ejecutarInstruccion(char* str){
	int offset = sizeof(uint32_t);
	logTrace("Instr: %s",str+offset);
	return strdup(str+offset);
}
