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
#include "interfazCPU.h"

int pidActual = 1;

//probando
PCB* buildNewPCB(int consolaFD){
	PCB *new = malloc(sizeof(PCB));
	new->processID = getNextPID();
	new->consolaFD = consolaFD;
	new->programCounter = 0;
	new->pagesQty = 10;			//ejemplo
	new->executedQuantums = 0;
	new->consolaActiva = true;
	logTrace("Creado PCB [PID:%d, ConsolaFD:%d, QuantumsExec:%d]",new->processID,new->consolaFD,new->executedQuantums);
	return new;
}

void destroyPCB(PCB* self){
	logTrace("Destruyendo PCB [PID:%d, ConsolaFD:%d]",self->processID,self->consolaFD);
	free(self->codeIndex);
	free(self->programa);
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

void abortFromREADY(Estados* estados,int index){
	PCB* pcb = list_remove(estados->ready->elements,index);
	logTrace("Plan: PCB:%d / READY -> abort",pcb->processID);
	sendToEXIT(pcb,estados);
}

void abortFromBLOCK(Estados* estados,int index){
	PCB* pcb = list_remove(estados->block->elements,index);
	logTrace("Plan: PCB:%d / BLOCK -> abort",pcb->processID);
	sendToEXIT(pcb,estados);
}

void abortFromNEW(Estados* estados,int index){
	PCB* pcb = list_remove(estados->new->elements,index);
	logTrace("Plan: PCB:%d / NEW -> abort",pcb->processID);
	sendToEXIT(pcb,estados);
}

void abortFromEXEC(Estados* estados,int pid){
	PCB* pcb = removeFromEXEC(estados,pid);
	logTrace("Plan: PCB:%d / EXEC -> abort",pcb->processID);
	sendToEXIT(pcb,estados);
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

PCB* removeNextFromEXIT(Estados* estados){
	PCB* pcb = queue_pop(estados->exit);
	logTrace("Plan: PCB:%d / EXIT -> fin",pcb->processID);
	return pcb;
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

int incrementarContadorPrograma(PCB* proceso){//TODO: esto lo tiene que hacer la CPU despues
	proceso->programCounter++;
	logTrace("Plan: PCB:%d / Program Counter: %d/%d",proceso->processID,proceso->programCounter,proceso->codeIndexLength);
	return proceso->codeIndexLength-proceso->programCounter;
}

void quantumFinishedCallback(Estados* estados, int pid, int quantum, int socketCPU){
	PCB* proceso = getFromEXEC(estados,pid);
	if(proceso!=NULL){
		if(proceso->consolaActiva){
			if(incrementarContadorPrograma(proceso)<=0){
				finalizarPrograma(estados,proceso->processID,socketCPU);
			} else {
				//si se le terminaron los quantums al proceso
				if(addQuantumToExecProcess(proceso,quantum)<=0){
					proceso->executedQuantums=0;//reinicio los quantums ejecutados
					switchProcess(estados,pid,socketCPU);
					sendFromEXECtoREADY(estados,pid);
				} else {
					continueExec(socketCPU,proceso);
				}
			}
		} else {
			//entra por aca si la consola cerro la conexion con el Nucleo
			logTrace("Consola estaba inactiva.");
			abortProcess(estados,pid,socketCPU);
		}
	}
}

void finalizarPrograma(Estados* estados, int pid, int socketCPU){
	switchProcess(estados,pid,socketCPU);
	PCB* proceso = removeFromEXEC(estados,pid);
	sendToEXIT(proceso,estados);
	//TODO: faltaria informarle a la consola que finalizo el programa
}

void switchProcess(Estados* estados, int pid, int socketCPU){
	logTrace("Informando CPU [Switch process]");
	informarCPU(socketCPU,ABORT_EXECUTION,pid);
}

void abortProcess(Estados* estados, int pid, int socketCPU){
	abortFromEXEC(estados,pid);
	informarCPU(socketCPU,ABORT_EXECUTION,pid);
}

void continueExec(int socketCPU, PCB* pcb){
	logTrace("Informando CPU [Continue process execution]");
	informarEjecucionCPU(socketCPU,CONTINUE_EXECUTION,pcb);
}

void startExec(Estados* estados, int socketCPU){
	PCB* proceso = getNextFromREADY(estados);
	sendToEXEC(proceso,estados);
	logTrace("Informando CPU [Execute new process]");
	informarEjecucionCPU(socketCPU,EXEC_NEW_PROCESS,proceso);
}

void informarEjecucionCPU(int socketCPU, int accion, PCB* pcb){
	char* instruccion = getSiguienteInstruccion(pcb);
	char* serialized = serializar_EjecutarInstruccion(pcb->processID,instruccion);
	int longitud = getLong_EjecutarInstruccion(instruccion);
	enviarMensajeSocketConLongitud(socketCPU,EXEC_NEW_PROCESS,serialized,longitud);
	free(serialized);
}

void informarCPU(int socketCPU, int accion, int pid){
	enviarMensajeSocket(socketCPU,accion,string_itoa(pid));
}

void iniciarPrograma(Estados* estados, int consolaFD, int socketPlanificador, char* programa){
	logTrace("Iniciando nuevo Programa Consola");
	PCB* nuevo = buildNewPCB(consolaFD);
	getCodeIndex(nuevo,programa);
	nuevo->programa = strdup(programa);
	sendToNEW(nuevo,estados);
	//TODO: hacer las validaciones para ver si puede pasar a READY
	//hay que ver si hacer que apenas entran se pongan en READY o si poner en NEW y que otra funcion vaya pasando los NEW a READY
	nuevo = getNextFromNEW(estados);
	sendToREADY(nuevo,estados);
	logTrace("Informando Planificador [Program READY]");
	informarPlanificador(socketPlanificador,PROGRAM_READY,nuevo->processID);
}

void abortarPrograma(Estados* estados, int consolaFD){
	logTrace("Finalizando Programa Consola");
	//primero lo busca en los estados distintos de EXEC(ejecutandose)
	bool encontrado = findAndExitPCBnotExecuting(estados,consolaFD);
	//si no lo encuentra, lo busca en la lista de EXEC
	if(!encontrado){
		findAndExitPCBexecuting(estados,consolaFD);
	}
}

bool findAndExitPCBnotExecuting(Estados* estados, int consolaFD){
	int i;
	PCB* pcb;
	//busco en ready
	for(i=0; i<estados->ready->elements->elements_count; i++){
		pcb = list_get(estados->ready->elements,i);
		if(pcb->consolaFD==consolaFD){
			pcb->consolaActiva = false;
			abortFromREADY(estados,i);
			return true;
		}
	}
	//busco en block
	for(i=0; i<estados->block->elements->elements_count; i++){
		pcb = list_get(estados->block->elements,i);
		if(pcb->consolaFD==consolaFD){
			pcb->consolaActiva = false;
			abortFromBLOCK(estados,i);
			return true;
		}
	}
	//busco en new
	for(i=0; i<estados->new->elements->elements_count; i++){
		pcb = list_get(estados->new->elements,i);
		if(pcb->consolaFD==consolaFD){
			pcb->consolaActiva = false;
			abortFromNEW(estados,i);
			return true;
		}
	}
	return false;
}

void findAndExitPCBexecuting(Estados* estados, int consolaFD){
	int i;
	PCB* pcb;
	//busco en EXEC
	for(i=0; i<estados->execute->elements_count; i++){
		pcb = list_get(estados->execute,i);
		if(pcb->consolaFD==consolaFD){
			pcb->consolaActiva = false;
			//otro metodo se encarga de validar este flag
			//para abortarlo cuando termine la ejecucion actual
		}
	}
}

void informarPlanificador(int socketPlanificador, int accion, int pid){
	enviarMensajeSocket(socketPlanificador,accion,string_itoa(pid));
}

void getCodeIndex(PCB* pcb, char* programa){
	char* aux = programa;
	ParCodigo* tabla = NULL;
	int i = 0;
	int tablaLen = 0;
	int ultimo = 0;

	while(*aux){
		if(*aux=='\n'){
			if(esInstruccionValida(programa,ultimo,i-ultimo)){
				tabla = (ParCodigo*) realloc(tabla, sizeof(ParCodigo)*(tablaLen+1));
				tabla[tablaLen].offset = ultimo;
				tabla[tablaLen].length = i-ultimo;
				tablaLen++;
			}
			ultimo = i+1;
		}
		aux++;
		i++;
	}
	pcb->codeIndex = tabla;
	pcb->codeIndexLength = tablaLen;
}

int esInstruccionValida(char* str, int offset, int length){
	int esValida;
	char* instruccion = malloc(sizeof(char)*length+1);
	if(instruccion!=NULL){
		memcpy(instruccion,str+offset,length);
		instruccion[length]='\0';

		if(string_starts_with(instruccion,"#")){
			esValida = 0;
		} else if(string_equals_ignore_case(instruccion,"begin")){
			esValida = 0;
		} else if(string_equals_ignore_case(instruccion,"end")){
			esValida = 0;
		} else {
			esValida = 1;
		}
		free(instruccion);
		return esValida;
	} else {
		return 0;
	}
}

char* getInstruccion(char* codigo, int offset, int length){
	char* instruccion = malloc(sizeof(char)*length+1);
	if(instruccion!=NULL){
		memcpy(instruccion,codigo+offset,length);
		instruccion[length]='\0';
	}
	return instruccion;
}

char* getSiguienteInstruccion(PCB* pcb){
	int offset = pcb->codeIndex[pcb->programCounter].offset;
	int length = pcb->codeIndex[pcb->programCounter].length;
	return getInstruccion(pcb->programa,offset,length);
}

