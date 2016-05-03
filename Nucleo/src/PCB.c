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
#include <mllibs/log/logger.h>
#include "planificador.h"

int pidActual = 0;

//probando
PCB* buildNewPCB(int consolaFD){
	PCB *new = malloc(sizeof(PCB));
	new->processID = getNextPID();
	new->consolaFD = consolaFD;
	new->programCounter = 2;	//ejemplo
	new->pagesQty = 10;			//ejemplo
	new->executedQuantums = 0;
	logTrace("Creado PCB [PID:%d, ConsolaFD:%d, QuantumsExec:%d]",new->processID,new->consolaFD,new->executedQuantums);
	return new;
}

void destroyPCB(PCB* self){
	logTrace("Destruyendo PCB [PID:%d, ConsolaFD:%d]",self->processID,self->consolaFD);
	free(self->codeIndex);
	//free(self->stackIndex);
	//free(self->tagIndex);
	free(self);
}

int getNextPID(){
	return pidActual++;
}

Estados* inicializarEstados(){
	logTrace("Inicializando Estados del Planificador");
	Estados* estados = malloc(sizeof(Estados));
	estados->block = queue_create();
	estados->execute = list_create();
	estados->exit = queue_create();
	estados->new = queue_create();
	estados->ready = queue_create();
	return estados;
}

void sendToNEW(PCB* pcb, Estados* estados){
	logTrace("Plan: PCB:%d / -> NEW",pcb->processID);
	queue_push(estados->new,pcb);
}

PCB* getNextFromNEW(Estados* estados){
	PCB* pcb = queue_pop(estados->new);
	logTrace("Plan: PCB:%d / NEW -> next",pcb->processID);
	return pcb;
}

PCB* getNextFromREADY(Estados* estados){
	PCB* pcb = queue_pop(estados->ready);
	logTrace("Plan: PCB:%d / READY -> next",pcb->processID);
	return pcb;
}

void sendToREADY(PCB* pcb, Estados* estados){
	logTrace("Plan: PCB:%d / -> READY",pcb->processID);
	queue_push(estados->ready,pcb);
}

void sendToEXEC(PCB* pcb, Estados* estados){
	logTrace("Plan: PCB:%d / -> EXEC",pcb->processID);
	list_add(estados->execute,pcb);
}
//TODO: ver si hay que organizar distintas colas de bloqueados
void sendToBLOCK(PCB* pcb, Estados* estados){
	logTrace("Plan: PCB:%d / -> BLOCK",pcb->processID);
	queue_push(estados->block,pcb);
}

void sendToEXIT(PCB* pcb, Estados* estados){
	logTrace("Plan: PCB:%d / -> EXIT",pcb->processID);
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
			logTrace("Plan: PCB:%d / EXEC ->",pid);
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

int addQuantumToExecProcess(PCB* proceso, int quantum){
	//le suma 1 a los quantums ejecutados
	proceso->executedQuantums++;
	logTrace("Plan: PCB:%d / Ejecutado 1 Quantum / Actual: %d/%d",proceso->processID,proceso->executedQuantums,quantum);
	//retorna la cantidad que le quedan por ejecutar
	return quantum - proceso->executedQuantums;
}

void quantumFinishedCallback(Estados* estados, int pid, int quantum, int socketCPU){
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
	logTrace("Informando CPU [Switch process]");
	informarCPU(socketCPU,ABORT_EXECUTION,pid);
}

void continueExec(int socketCPU,int pid){
	logTrace("Informando CPU [Continue process execution]");
	informarCPU(socketCPU,CONTINUE_EXECUTION,pid);
}

void startExec(Estados* estados, int socketCPU){
	PCB* proceso = getNextFromREADY(estados);
	sendToEXEC(proceso,estados);
	logTrace("Informando CPU [Execute new process]");
	informarCPU(socketCPU,EXEC_NEW_PROCESS,proceso->processID);
}

void informarCPU(int socketCPU, int accion, int pid){
	Package package;
	fillPackage(&package,accion,string_itoa(pid));
	char* serializedPkg = serializarMensaje(&package);
	escribirSocketServer(socketCPU, (char *)serializedPkg, getLongitudPackage(&package));
}

void iniciarPrograma(Estados* estados, int consolaFD, int socketPlanificador){
	logTrace("Iniciando nuevo Programa Consola");
	PCB* nuevo = buildNewPCB(consolaFD);
	sendToNEW(nuevo,estados);
	//TODO: hacer las validaciones para ver si puede pasar a READY
	//hay que ver si hacer que apenas entran se pongan en READY o si poner en NEW y que otra funcion vaya pasando los NEW a READY
	nuevo = getNextFromNEW(estados);
	sendToREADY(nuevo,estados);
	logTrace("Informando Planificador [Program READY]");
	informarPlanificador(socketPlanificador,PROGRAM_READY,nuevo->processID);
}

void informarPlanificador(int socketPlanificador, int accion, int pid){
	Package package;
	fillPackage(&package,accion,string_itoa(pid));
	char* serializedPkg = serializarMensaje(&package);
	escribirSocketServer(socketPlanificador, (char *)serializedPkg, getLongitudPackage(&package));
}

