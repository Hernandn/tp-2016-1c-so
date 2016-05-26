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

int pidActual = 1;

//mutex de colas/listas de estados
pthread_mutex_t blockMutex;
pthread_mutex_t executeMutex;
pthread_mutex_t exitMutex;
pthread_mutex_t newMutex;
pthread_mutex_t readyMutex;

//probando
PCB* buildNewPCB(int consolaFD, char* programa){
	PCB *new = malloc(sizeof(PCB));
	new->processID = getNextPID();
	new->consolaFD = consolaFD;
	new->programCounter = 0;
	new->pagesQty = 10;			//ejemplo
	new->executedQuantums = 0;
	new->consolaActiva = true;
	new->codeIndex = metadata_desde_literal(programa);
	new->programa = strdup(programa);
	logTrace("Creado PCB [PID:%d, ConsolaFD:%d, QuantumsExec:%d]",new->processID,new->consolaFD,new->executedQuantums);
	return new;
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

	//inicializo los mutex
	pthread_mutex_init(&blockMutex,NULL);
	pthread_mutex_init(&executeMutex,NULL);
	pthread_mutex_init(&exitMutex,NULL);
	pthread_mutex_init(&newMutex,NULL);
	pthread_mutex_init(&readyMutex,NULL);

	return estados;
}

void destroyEstados(Estados* estados){
	queue_destroy_and_destroy_elements(estados->block,(void*)destroyPCB);
	list_destroy_and_destroy_elements(estados->execute,(void*)destroyPCB);
	queue_destroy_and_destroy_elements(estados->exit,(void*)destroyPCB);
	queue_destroy_and_destroy_elements(estados->new,(void*)destroyPCB);
	queue_destroy_and_destroy_elements(estados->ready,(void*)destroyPCB);
	pthread_mutex_destroy(&blockMutex);
	pthread_mutex_destroy(&executeMutex);
	pthread_mutex_destroy(&exitMutex);
	pthread_mutex_destroy(&newMutex);
	pthread_mutex_destroy(&readyMutex);
}

void sendToNEW(PCB* pcb, Estados* estados){
	pthread_mutex_lock(&newMutex);
	queue_push(estados->new,pcb);
	pthread_mutex_unlock(&newMutex);
	logTrace("Plan: PCB:%d / -> NEW",pcb->processID);
}

PCB* getNextFromNEW(Estados* estados){
	pthread_mutex_lock(&newMutex);
	PCB* pcb = queue_pop(estados->new);
	pthread_mutex_unlock(&newMutex);
	logTrace("Plan: PCB:%d / NEW -> next",pcb->processID);
	return pcb;
}

PCB* getNextFromREADY(Estados* estados){
	pthread_mutex_lock(&readyMutex);
	PCB* pcb = queue_pop(estados->ready);
	pthread_mutex_unlock(&readyMutex);
	logTrace("Plan: PCB:%d / READY -> next",pcb->processID);
	return pcb;
}

void sendToREADY(PCB* pcb, Estados* estados){
	pthread_mutex_lock(&readyMutex);
	queue_push(estados->ready,pcb);
	pthread_mutex_unlock(&readyMutex);
	logTrace("Plan: PCB:%d / -> READY",pcb->processID);
}

void sendToEXEC(PCB* pcb, Estados* estados){
	pthread_mutex_lock(&executeMutex);
	list_add(estados->execute,pcb);
	pthread_mutex_unlock(&executeMutex);
	logTrace("Plan: PCB:%d / -> EXEC",pcb->processID);
}
//TODO: ver si hay que organizar distintas colas de bloqueados
void sendToBLOCK(PCB* pcb, Estados* estados){
	pthread_mutex_lock(&blockMutex);
	queue_push(estados->block,pcb);
	pthread_mutex_unlock(&blockMutex);
	logTrace("Plan: PCB:%d / -> BLOCK",pcb->processID);
}

void sendToEXIT(PCB* pcb, Estados* estados){
	pthread_mutex_lock(&exitMutex);
	queue_push(estados->exit,pcb);
	pthread_mutex_unlock(&exitMutex);
	logTrace("Plan: PCB:%d / -> EXIT",pcb->processID);
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
	bool encontrado = false;
	pthread_mutex_lock(&executeMutex);
	t_list* enEjecucion = estados->execute;
	PCB* proceso = NULL;
	for( i=0; i < enEjecucion->elements_count; i++){
		proceso = list_get(enEjecucion,i);
		//saca de la lista y retorna el proceso cuando lo encuentra por el ID
		if(proceso->processID==pid){
			list_remove(enEjecucion,i);
			encontrado = true;
			break;
		}
	}
	pthread_mutex_unlock(&executeMutex);
	logTrace("Plan: PCB:%d / EXEC ->",pid);
	if(encontrado){
		return proceso;
	} else {
		return NULL;
	}
}

PCB* removeNextFromEXIT(Estados* estados){
	pthread_mutex_lock(&exitMutex);
	PCB* pcb = queue_pop(estados->exit);
	pthread_mutex_unlock(&exitMutex);
	logTrace("Plan: PCB:%d / EXIT -> fin",pcb->processID);
	return pcb;
}

PCB* getFromEXEC(Estados* estados, int pid){
	int i;
	bool encontrado = false;
	pthread_mutex_lock(&executeMutex);
	t_list* enEjecucion = estados->execute;
	PCB* proceso = NULL;
	for( i=0; i < enEjecucion->elements_count; i++){
		proceso = list_get(enEjecucion,i);
		//retorna el proceso cuando lo encuentra por el ID
		if(proceso->processID==pid){
			encontrado = true;
			break;
		}
	}
	pthread_mutex_unlock(&executeMutex);
	if(encontrado){
		return proceso;
	} else {
		return NULL;
	}
}

bool hayProcesosEnREADY(Estados* estados){
	return (estados->ready->elements->elements_count)>0;
}

int addQuantumToExecProcess(PCB* proceso, int quantum){
	//le suma 1 a los quantums ejecutados
	proceso->executedQuantums++;
	logTrace("Plan: PCB:%d / Ejecutado 1 Quantum / Actual: %d/%d",proceso->processID,proceso->executedQuantums,quantum);
	//retorna la cantidad que le quedan por ejecutar
	return quantum - proceso->executedQuantums;
}

void quantumFinishedCallback(Estados* estados, int pid, int quantum, int socketCPU, int socketPlanificador){
	PCB* proceso = getFromEXEC(estados,pid);
	if(proceso!=NULL){
		if(proceso->consolaActiva){
			//si se le terminaron los quantums al proceso
			if(addQuantumToExecProcess(proceso,quantum)<=0){
				proceso->executedQuantums=0;//reinicio los quantums ejecutados
				switchProcess(estados,pid,socketCPU);
			} else {
				continueExec(socketCPU,proceso);
			}
		} else {
			//entra por aca si la consola cerro la conexion con el Nucleo
			logTrace("Consola estaba inactiva.");
			abortProcess(estados,pid,socketCPU);
		}
	}
}

void contextSwitchFinishedCallback(Estados* estados, PCB* pcbActualizado, int socketPlanificador){
	PCB* proceso = removeFromEXEC(estados,pcbActualizado->processID);
	if(proceso!=NULL){
		actualizarPCB(proceso,pcbActualizado);
		logTrace("Context Switch finished callback: PCB:%d / PC: %d/%d",proceso->processID,proceso->programCounter,proceso->codeIndex->instrucciones_size);
		sendToREADY(proceso,estados);
		logTrace("Informando Planificador [Program READY]");
		informarPlanificador(socketPlanificador,PROGRAM_READY,proceso->processID);
	}
}

void finalizarPrograma(Estados* estados, PCB* pcbActualizado, int socketCPU, int socketPlanificador){
	PCB* proceso = removeFromEXEC(estados,pcbActualizado->processID);
	actualizarPCB(proceso,pcbActualizado);
	destroyPCB(pcbActualizado);
	sendToEXIT(proceso,estados);
	enviarMensajeSocket(socketPlanificador,FINALIZAR_PROGRAMA,"");
}

void actualizarPCB(PCB* local, PCB* actualizado){
	local->programCounter = actualizado->programCounter;
}

void switchProcess(Estados* estados, int pid, int socketCPU){
	logTrace("Informando CPU [Switch process]");
	informarCPU(socketCPU,CONTEXT_SWITCH,pid);
}

void abortProcess(Estados* estados, int pid, int socketCPU){
	abortFromEXEC(estados,pid);
	informarCPU(socketCPU,ABORT_EXECUTION,pid);
}

void continueExec(int socketCPU, PCB* pcb){
	logTrace("Informando CPU [Continue process execution]");
	continuarEjecucionProcesoCPU(socketCPU);
}

void startExec(Estados* estados, int socketCPU){
	PCB* proceso = getNextFromREADY(estados);
	sendToEXEC(proceso,estados);
	logTrace("Informando CPU [Execute new process]");
	ejecutarNuevoProcesoCPU(socketCPU,proceso);
}
/*
void informarEjecucionCPU(int socketCPU, int accion, PCB* pcb){
	char* instruccion = getSiguienteInstruccion(pcb);
	char* serialized = serializar_EjecutarInstruccion(pcb->processID,instruccion);
	int longitud = getLong_EjecutarInstruccion(instruccion);
	enviarMensajeSocketConLongitud(socketCPU,EXEC_NEW_PROCESS,serialized,longitud);
	free(serialized);
}*/

void informarCPU(int socketCPU, int accion, int pid){
	enviarMensajeSocket(socketCPU,accion,string_itoa(pid));
}

void iniciarPrograma(Estados* estados, int consolaFD, int socketPlanificador, char* programa){
	logTrace("Iniciando nuevo Programa Consola");
	PCB* nuevo = buildNewPCB(consolaFD,programa);
	sendToNEW(nuevo,estados);

	//esto es para pedirle a la UMC que reserve espacio para el programa
	int socketUMC = getSocketUMC();
	int size_pagina = getConfiguration()->size_pagina;
	int stack_size = getConfiguration()->stack_size;
	int pagsNecesarias = getCantidadPaginasNecesarias(programa,size_pagina,stack_size);
	logDebug("Se necesitan %d paginas para almacenar el programa",pagsNecesarias);

	//pagina* paginas = getPaginasFromPrograma(programa,size_pagina);
	//destroyPaginas(paginas,pagsNecesarias-stack_size);
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
	bool encontrado = false;
	//busco en ready
	pthread_mutex_lock(&readyMutex);
	for(i=0; i<estados->ready->elements->elements_count; i++){
		pcb = list_get(estados->ready->elements,i);
		if(pcb->consolaFD==consolaFD){
			pcb->consolaActiva = false;
			abortFromREADY(estados,i);
			encontrado = true;
			break;
		}
	}
	pthread_mutex_unlock(&readyMutex);

	if(!encontrado){
		//busco en block
		pthread_mutex_lock(&blockMutex);
		for(i=0; i<estados->block->elements->elements_count; i++){
			pcb = list_get(estados->block->elements,i);
			if(pcb->consolaFD==consolaFD){
				pcb->consolaActiva = false;
				abortFromBLOCK(estados,i);
				encontrado = true;
				break;
			}
		}
		pthread_mutex_unlock(&blockMutex);
	}

	if(!encontrado){
		//busco en new
		pthread_mutex_lock(&newMutex);
		for(i=0; i<estados->new->elements->elements_count; i++){
			pcb = list_get(estados->new->elements,i);
			if(pcb->consolaFD==consolaFD){
				pcb->consolaActiva = false;
				abortFromNEW(estados,i);
				encontrado = true;
				break;
			}
		}
		pthread_mutex_unlock(&newMutex);
	}

	return encontrado;
}

void findAndExitPCBexecuting(Estados* estados, int consolaFD){
	int i;
	PCB* pcb;
	//busco en EXEC
	pthread_mutex_lock(&executeMutex);
	for(i=0; i<estados->execute->elements_count; i++){
		pcb = list_get(estados->execute,i);
		if(pcb->consolaFD==consolaFD){
			pcb->consolaActiva = false;
			//otro metodo se encarga de validar este flag
			//para abortarlo cuando termine la ejecucion actual
		}
	}
	pthread_mutex_unlock(&executeMutex);
}

void informarPlanificador(int socketPlanificador, int accion, int pid){
	enviarMensajeSocket(socketPlanificador,accion,string_itoa(pid));
}

/*
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
*/

int getCantidadPaginasPrograma(char* programa, int size_pagina){
	int length = strlen(programa);
	int cantidad = length/size_pagina;
	if(length%size_pagina!=0){
		cantidad++;//agrego una pagina mas para lo que resta
	}
	return cantidad;
}

int getCantidadPaginasNecesarias(char* programa, int size_pagina, int stack_size){
	int cantidad = getCantidadPaginasPrograma(programa,size_pagina);
	cantidad += stack_size;
	return cantidad;
}

pagina* getPaginasFromPrograma(char* programa, int size_pagina){
	int cantPags = getCantidadPaginasPrograma(programa,size_pagina);
	int offset = 0;
	pagina* pags = malloc(sizeof(pagina)*cantPags);
	int i;
	for(i=0; i<cantPags; i++){
		pags[i] = malloc(sizeof(char)*size_pagina);
		memcpy(pags[i],programa+offset,size_pagina);
		offset+=size_pagina;
	}
	return pags;
}

void destroyPaginas(pagina* paginas, int cantidad){
	int i;
	for(i=0; i<cantidad; i++){
		free(paginas[i]);
	}
	free(paginas);
}


