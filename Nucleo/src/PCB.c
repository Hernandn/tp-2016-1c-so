/*
 * PCB.c
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "PCB.h"
#include "Nucleo.h"
#include <commons/string.h>
#include <mllibs/sockets/server.h>
#include <mllibs/sockets/package.h>
#include <mllibs/sockets/client.h>

int pidActual = 0;

//probando
PCB* buildNewPCB(){
	PCB *new = malloc(sizeof(PCB));
	new->processID = getNextPID();
	new->programCounter = 2;	//ejemplo
	new->pagesQty = 10;			//ejemplo
	new->executedQuantums = 0;
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
	estados->execute = list_create();
	estados->exit = queue_create();
	estados->new = queue_create();
	estados->ready = queue_create();
}

void sendToNEW(PCB* pcb, Estados* estados){
	queue_push(estados->new,pcb);
}

PCB* getNextFromNEW(Estados* estados){
	return queue_pop(estados->new);
}

PCB* getNextFromREADY(Estados* estados){
	return queue_pop(estados->ready);
}

void sendToREADY(PCB* pcb, Estados* estados){
	queue_push(estados->ready,pcb);
}

void sendToEXEC(PCB* pcb, Estados* estados){
	list_add(estados->execute,pcb);
}
//TODO: ver si hay que organizar distintas colas de bloqueados
void sendToBLOCK(PCB* pcb, Estados* estados){
	queue_push(estados->block,pcb);
}

void sendToEXIT(PCB* pcb, Estados* estados){
	queue_push(estados->exit,pcb);
}

void sendFromEXECtoREADY(Estados* estados, int pid){
	PCB* proceso = removeFromEXEC(estados,pid);
	if(proceso!=NULL){
		sendToREADY(proceso,estados);
	}
}

PCB* removeFromEXEC(Estados* estados, int pid){
	int i;
	t_list* enEjecucion = estados->execute;
	PCB* proceso = NULL;
	for( i=0; i < enEjecucion->elements_count; i++){
		proceso = list_get(enEjecucion,i);
		//saca de la lista y retorna el proceso cuando lo encuentra por el ID
		if(proceso->processID==pid){
			list_remove(enEjecucion,i);
			return proceso;
		}
	}
	return NULL;
}

PCB* getFromEXEC(Estados* estados, int pid){
	int i;
	t_list* enEjecucion = estados->execute;
	PCB* proceso = NULL;
	for( i=0; i < enEjecucion->elements_count; i++){
		proceso = list_get(enEjecucion,i);
		//retorna el proceso cuando lo encuentra por el ID
		if(proceso->processID==pid){
			return proceso;
		}
	}
	return NULL;
}

int addQuantumToExecProcess(PCB* proceso, int* quantum){
	//le suma 1 a los quantums ejecutados
	proceso->executedQuantums++;
	//retorna la cantidad que le quedan por ejecutar
	return *quantum - proceso->executedQuantums;
}

void quantumFinishedCallback(Estados* estados, int pid, int* quantum, int socketCPU){
	PCB* proceso = getFromEXEC(estados,pid);
	if(proceso!=NULL){
		//si se le terminaron los quantums al proceso
		if(addQuantumToExecProcess(proceso,quantum)<=0){
			switchProcess(estados,pid,socketCPU);
		} else {
			continueExec(socketCPU,pid);
		}
	}
}

void switchProcess(Estados* estados, int pid, int socketCPU){
	sendFromEXECtoREADY(estados,pid);
	informarCPU(socketCPU,ABORT_EXECUTION,pid);
}

void continueExec(int socketCPU,int pid){
	informarCPU(socketCPU,CONTINUE_EXECUTION,pid);
}

void informarCPU(int socketCPU, int accion, int pid){
	Package package;
	fillPackage(&package,accion,string_itoa(pid));
	char* serializedPkg = serializarMensaje(&package);
	escribirSocketServer(socketCPU, (char *)serializedPkg, getLongitudPackage(&package));
}

