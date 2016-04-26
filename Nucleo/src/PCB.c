/*
 * PCB.c
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "PCB.h"

int pidActual = 0;

//probando
PCB* buildNewPCB(){
	PCB *new = malloc(sizeof(PCB));
	new->processID = getNextPID();
	new->programCounter = 2;
	new->pagesQty = 10;
	return new;
}

void destroyPCB(PCB* self){
	free(self->codeIndex);
	//free(self->stackIndex);
	//free(self->tagIndex);
	free(self);
}

int getNextPID(){
	return pidActual++;
}

void inicializarEstados(Estados* estados){
	estados->block = queue_create();
	estados->execute = queue_create();
	estados->exit = queue_create();
	estados->new = queue_create();
	estados->ready = queue_create();
}

