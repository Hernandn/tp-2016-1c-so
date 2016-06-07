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
#include <mllibs/umc/interfaz.h>
#include <mllibs/log/logger.h>
#include <mllibs/stack/stack.h>
#include <semaphore.h>
#include "planificador.h"

int pidActual = 1;

//mutex de colas/listas de estados
pthread_mutex_t executeMutex;
pthread_mutex_t exitMutex;
pthread_mutex_t newMutex;
pthread_mutex_t readyMutex;

//array de mutex para cada dispositivo de entrada/salida (para ejecutar la instruccion IO)
pthread_mutex_t* io_mutex_array;
//array de mutex para cada dispositivo de entrada/salida (para encolar/desencolar en bloqueados)
pthread_mutex_t* io_mutex_queues;
//array de semaforos para cada dispositivo de entrada/salida
sem_t* io_sem_array;

//probando
PCB* buildNewPCB(int consolaFD, char* programa){
	PCB *new = malloc(sizeof(PCB));
	new->processID = getNextPID();
	new->consolaFD = consolaFD;
	new->stackFirstPage = 0;
	new->stackOffset = 0;
	new->programCounter = 0;
	new->executedQuantums = 0;
	new->consolaActiva = true;
	new->codeIndex = metadata_desde_literal(programa);

	printf("Instruccion inicio %d\n",new->codeIndex->instruccion_inicio);
	printf("Instruccion size %d\n",new->codeIndex->instrucciones_size);
	int i;
	for(i=0; i<new->codeIndex->instrucciones_size; i++){
		printf("Instruccion ini %d  offset %d\n",new->codeIndex->instrucciones_serializado[i].start, new->codeIndex->instrucciones_serializado[i].offset);
	}
	printf("cant etiquetas %d\n",new->codeIndex->cantidad_de_etiquetas);
	printf("cant funciones %d\n",new->codeIndex->cantidad_de_funciones);
	printf("etiquetas size %d\n",new->codeIndex->etiquetas_size);
	printf("etiquetas %s\n",new->codeIndex->etiquetas);

	new->context_len = 0;
	new->stackIndex = NULL;
	crearNuevoContexto(new);//inicializo el contexto del "main"
	new->programa = strdup(programa);//TODO: borrar
	logTrace("Creado PCB [PID:%d, ConsolaFD:%d, QuantumsExec:%d]",new->processID,new->consolaFD,new->executedQuantums);
	return new;
}

int getNextPID(){
	return pidActual++;
}

Estados* inicializarEstados(){
	logTrace("Inicializando Estados del Planificador");
	Estados* estados = malloc(sizeof(Estados));
	//inicializo colas de bloqueados (1 por dispositivo)
	estados->block = malloc(sizeof(t_queue*)*config->io_length);
	int i;
	for(i=0; i<config->io_length; i++){
		estados->block[i] = queue_create();
	}

	estados->execute = list_create();
	estados->exit = queue_create();
	estados->new = queue_create();
	estados->ready = queue_create();

	io_mutex_queues = malloc(sizeof(pthread_mutex_t)*config->io_length);
	//inicializo los mutex
	for(i=0; i<config->io_length; i++){
		pthread_mutex_init(&io_mutex_queues[i],NULL);
	}
	pthread_mutex_init(&executeMutex,NULL);
	pthread_mutex_init(&exitMutex,NULL);
	pthread_mutex_init(&newMutex,NULL);
	pthread_mutex_init(&readyMutex,NULL);

	return estados;
}

void destroyEstados(Estados* estados){
	int i;
	for(i=0; i<config->io_length; i++){
		queue_destroy_and_destroy_elements(estados->block[i],(void*)destroy_solicitud_io);
	}
	list_destroy_and_destroy_elements(estados->execute,(void*)destroyPCB);
	queue_destroy_and_destroy_elements(estados->exit,(void*)destroyPCB);
	queue_destroy_and_destroy_elements(estados->new,(void*)destroyPCB);
	queue_destroy_and_destroy_elements(estados->ready,(void*)destroyPCB);
	for(i=0; i<config->io_length; i++){
		pthread_mutex_destroy(&io_mutex_queues[i]);
	}
	pthread_mutex_destroy(&executeMutex);
	pthread_mutex_destroy(&exitMutex);
	pthread_mutex_destroy(&newMutex);
	pthread_mutex_destroy(&readyMutex);
}

void destroy_solicitud_io(solicitud_io* self){
	destroyPCB(self->pcb);
	free(self->io_id);
	free(self);
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

solicitud_io* getNextFromBlock(Estados* estados, int io_index){
	pthread_mutex_lock(&io_mutex_queues[io_index]);
	solicitud_io* sol = queue_pop(estados->block[io_index]);
	pthread_mutex_unlock(&io_mutex_queues[io_index]);
	logTrace("Plan: PCB:%d / Block [%s] -> next",sol->pcb->processID,sol->io_id);
	return sol;
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
void sendToBLOCK(solicitud_io* solicitud, int io_index, Estados* estados){
	pthread_mutex_lock(&io_mutex_queues[io_index]);
	queue_push(estados->block[io_index],solicitud);
	pthread_mutex_unlock(&io_mutex_queues[io_index]);
	logTrace("Plan: PCB:%d / -> BLOCK",solicitud->pcb->processID);
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

void abortFromBLOCK(Estados* estados,int index, int io_index){
	solicitud_io* solicitud = list_remove(estados->block[io_index]->elements,index);
	logTrace("Plan: PCB:%d / BLOCK -> abort",solicitud->pcb->processID);
	sendToEXIT(solicitud->pcb,estados);
	free_solicitud_io(solicitud);
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

void quantumFinishedCallback(Estados* estados, int pid, int quantum, int socketCPU){
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

void contextSwitchFinishedCallback(Estados* estados, PCB* pcbActualizado){
	PCB* proceso = removeFromEXEC(estados,pcbActualizado->processID);
	if(proceso!=NULL){
		actualizarPCB(proceso,pcbActualizado);
		logTrace("Context Switch finished callback: PCB:%d / PC: %d/%d",proceso->processID,proceso->programCounter,proceso->codeIndex->instrucciones_size);
		notifyProcessREADY(estados, proceso);
	}
}

void notifyProcessREADY(Estados* estados, PCB* pcb){
	sendToREADY(pcb,estados);
	logTrace("Informando Planificador [Program READY]");
	informarPlanificador(PROGRAM_READY,pcb->processID);
}

void finalizarPrograma(Estados* estados, PCB* pcbActualizado, int socketCPU){
	PCB* proceso = removeFromEXEC(estados,pcbActualizado->processID);
	actualizarPCB(proceso,pcbActualizado);
	destroyPCB(pcbActualizado);
	sendToEXIT(proceso,estados);
	enviarMensajeSocket(socketPlanificador,FINALIZAR_PROGRAMA,"");
}

void actualizarPCB(PCB* local, PCB* actualizado){
	local->programCounter = actualizado->programCounter;
	local->stackOffset = actualizado->stackOffset;
	local->context_len = actualizado->context_len;
	contexto* contexto_aux = local->stackIndex;
	local->stackIndex = actualizado->stackIndex;
	actualizado->stackIndex = contexto_aux;
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

void iniciarPrograma(Estados* estados, int consolaFD, char* programa){
	logTrace("Iniciando nuevo Programa Consola");
	PCB* nuevo = buildNewPCB(consolaFD,programa);
	sendToNEW(nuevo,estados);

	//esto es para pedirle a la UMC que reserve espacio para el programa
	//int socketUMC = getSocketUMC();
	int size_pagina = getConfiguration()->size_pagina;
	int stack_size = getConfiguration()->stack_size;
	int pagsNecesarias = getCantidadPaginasNecesarias(programa,size_pagina,stack_size);
	nuevo-> stackFirstPage = pagsNecesarias;
	logDebug("Se necesitan %d paginas para almacenar el programa",pagsNecesarias);

	//pagina* paginas = getPaginasFromPrograma(programa,size_pagina);
	//destroyPaginas(paginas,pagsNecesarias-stack_size);
	//TODO: hacer las validaciones para ver si puede pasar a READY
	//hay que ver si hacer que apenas entran se pongan en READY o si poner en NEW y que otra funcion vaya pasando los NEW a READY
	nuevo = getNextFromNEW(estados);
	sendToREADY(nuevo,estados);
	logTrace("Informando Planificador [Program READY]");
	informarPlanificador(PROGRAM_READY,nuevo->processID);
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

	int j=0;
	while(!encontrado && j<config->io_length){
		//busco en block
		pthread_mutex_lock(&io_mutex_queues[i]);
		for(i=0; i<estados->block[i]->elements->elements_count; i++){
			solicitud_io* solicitud = list_get(estados->block[i]->elements,i);
			if(solicitud->pcb->consolaFD==consolaFD){
				solicitud->pcb->consolaActiva = false;
				abortFromBLOCK(estados,i,j);
				encontrado = true;
				break;
			}
		}
		pthread_mutex_unlock(&io_mutex_queues[i]);
		j++;
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

void informarPlanificador(int accion, int pid){
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

void launch_IO_threads(Estados* estados){
	pthread_t thread_io;

	io_sem_array = malloc(sizeof(sem_t)*config->io_length);
	io_mutex_array = malloc(sizeof(pthread_mutex_t)*config->io_length);

	int i;
	for(i=0; i<config->io_length; i++){
		io_arg_struct *args = malloc(sizeof(io_arg_struct));
		args->estados = estados;
		args->io_index = i;

		//inicializo el semaforo en 0 (vacio)
		sem_init(&io_sem_array[i],0,0);
		//inicializo mutex
		pthread_mutex_init(&io_mutex_array[i],NULL);

		pthread_create(&thread_io,NULL,(void*) ejecutarIO,(void*) args);
		logTrace("Creado thread para atender dispositivo: %s",config->io_ids[i]);
	}
}

void ejecutarIO(void* arguments){
	io_arg_struct *args = arguments;

	Estados* estados = args->estados;
	int io_index = args->io_index;

	while(1){

		sem_wait(&io_sem_array[io_index]);//consumo 1, se suspende el hilo aca si no hay ninguno esperando en cola de bloqueados para este dispositivo

		solicitud_io* solicitud = getNextFromBlock(estados,io_index);

		//operacion critica de I/O
		pthread_mutex_lock(&io_mutex_array[io_index]);

		logTrace("Ejecutando %d operaciones de %s (%d ms sleep)",solicitud->cant_operaciones,solicitud->io_id,config->io_sleep[io_index]);
		//hago un sleep del tiempo del dispositivo por la cantidad de operaciones
		usleep(config->io_sleep[io_index]*solicitud->cant_operaciones*1000);//paso de micro a milisegundos (*1000)

		pthread_mutex_unlock(&io_mutex_array[io_index]);

		//mando el proceso a ready
		notifyProcessREADY(estados, solicitud->pcb);

		//libero la solicitud
		free_solicitud_io(solicitud);
	}

	destroy_io_arg_struct(args);
}

void atenderSolicitudDispositivoIO(Estados* estados, solicitud_io* solicitud){

	int io_index = getPosicionDispositivo(config->io_ids,config->io_length,solicitud->io_id);
	PCB* pcb = removeFromEXEC(estados,solicitud->pcb->processID);
	actualizarPCB(pcb,solicitud->pcb);

	destroyPCB(solicitud->pcb);

	solicitud->pcb = pcb;

	sendToBLOCK(solicitud,io_index,estados);

	sem_post(&io_sem_array[io_index]);//incremento el semaforo
}

void destroy_io_arg_struct(io_arg_struct *args){
	free(args);
}

int getPosicionDispositivo(char** lista_ids, int len, char* io_id){
	int i=0;
	while(i<len && !string_equals_ignore_case(lista_ids[i],io_id)){
		i++;
	}
	return 0;
}

void free_solicitud_io(solicitud_io* solicitud){
	free(solicitud->io_id);
	free(solicitud);
}


