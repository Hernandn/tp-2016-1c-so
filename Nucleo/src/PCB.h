/*
 * PCB.h
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#ifndef PCB_H_
#define PCB_H_

#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <parser/metadata_program.h>


/*
 * Estructura PCB de un Proceso en el sistema
 */
typedef struct PCB {
	uint32_t processID;		//identificador unico del proceso
	int consolaFD;		//file descriptor del socket de la consola que inicio el programa
	uint32_t programCounter;	//contador de programa
	uint32_t pagesQty;		//cantidad de paginas de codigo ANSISOP
	int executedQuantums;	//cantidad de quantums ya ejecutados
	t_metadata_program* codeIndex;		//indice de codigo
	int* stackIndex;		//indice del stack (todavia no implementado)
	bool consolaActiva;	//indica si la consola esta activa o si cerro la conexion
	char* programa;	//codigo del programa
} PCB;

typedef char* pagina;

/*
 * Estructura con las colas de Estados
 */
typedef struct Estados {
	t_queue* new;
	t_queue* ready;
	t_list* execute;
	t_queue* block;
	t_queue* exit;
} Estados;



//funciones
PCB* buildNewPCB(int consolaFD, char* programa);
void destroyPCB(PCB* self);
int getNextPID();
Estados* inicializarEstados();
void sendToNEW(PCB* pcb, Estados* estados);
PCB* getNextFromNEW(Estados* estados);
PCB* getNextFromREADY(Estados* estados);
void sendToREADY(PCB* pcb, Estados* estados);
void sendToEXEC(PCB* pcb, Estados* estados);
void sendToBLOCK(PCB* pcb, Estados* estados);
void sendToEXIT(PCB* pcb, Estados* estados);
void sendFromEXECtoREADY(Estados* estados, int pid);
void abortFromREADY(Estados* estados,int index);
void abortFromBLOCK(Estados* estados,int index);
void abortFromNEW(Estados* estados,int index);
void abortFromEXEC(Estados* estados,int pid);
PCB* removeFromEXEC(Estados* estados, int pid);
PCB* removeNextFromEXIT(Estados* estados);
PCB* getFromEXEC(Estados* estados, int pid);
int addQuantumToExecProcess(PCB* proceso, int quantum);
void quantumFinishedCallback(Estados* estados, int pid, int quantum, int socketCPU, int socketPlanificador);
void contextSwitchFinishedCallback(Estados* estados, PCB* pcbActualizado, int socketPlanificador);
void finalizarPrograma(Estados* estados, PCB* pcb, int socketCPU, int socketPlanificador);
void actualizarPCB(PCB* local, PCB* actualizado);
void switchProcess(Estados* estados, int pid, int socketCPU);
void abortProcess(Estados* estados, int pid, int socketCPU);
void continueExec(int socketCPU, PCB* pcb);
void startExec(Estados* estados, int socketCPU);
void informarEjecucionCPU(int socketCPU, int accion, PCB* pcb);
void informarCPU(int socketCPU, int accion, int pid);
void iniciarPrograma(Estados* estados, int consolaFD, int socketPlanificador, char* programa);
void abortarPrograma(Estados* estados, int consolaFD);
bool findAndExitPCBnotExecuting(Estados* estados, int consolaFD);
void findAndExitPCBexecuting(Estados* estados, int consolaFD);
void informarPlanificador(int socketPlanificador, int accion, int pid);
void getCodeIndex(PCB* pcb, char* programa);
int esInstruccionValida(char* str, int offset, int length);
int getCantidadPaginasPrograma(char* programa, int size_pagina);
int getCantidadPaginasNecesarias(char* programa, int size_pagina, int stack_size);
pagina* getPaginasFromPrograma(char* programa, int size_pagina);
void destroyPaginas(pagina* paginas, int cantidad);

#endif /* PCB_H_ */
