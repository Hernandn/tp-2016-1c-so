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

/**
 * Estructura que representa la posicion de una linea
 * de codigo en un programa fuente ANSISOP
 */
typedef struct ParCodigo {
	int offset;	//byte a partir del cual empieza la linea de codigo
	int length;	//longitud de la linea de codigo
} ParCodigo;


/*
 * Estructura PCB de un Proceso en el sistema
 */
typedef struct PCB {
	int processID;		//identificador unico del proceso
	int consolaFD;		//file descriptor del socket de la consola que inicio el programa
	int programCounter;	//contador de programa
	int pagesQty;		//cantidad de paginas de codigo ANSISOP
	int executedQuantums;	//cantidad de quantums ya ejecutados
	ParCodigo* codeIndex;		//indice de codigo
	int* tagIndex;		//indice de etiquetas
	int* stackIndex;		//indice del stack
	bool consolaActiva;	//indica si la consola esta activa o si cerro la conexion
} PCB;


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
PCB* buildNewPCB(int consolaFD);
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
PCB* getFromEXEC(Estados* estados, int pid);
int addQuantumToExecProcess(PCB* proceso, int quantum);
void quantumFinishedCallback(Estados* estados, int pid, int quantum, int socketCPU);
void switchProcess(Estados* estados, int pid, int socketCPU);
void abortProcess(Estados* estados, int pid, int socketCPU);
void continueExec(int socketCPU,int pid);
void startExec(Estados* estados, int socketCPU);
void informarCPU(int socketCPU, int accion, int pid);
void iniciarPrograma(Estados* estados, int consolaFD, int socketPlanificador);
void finalizarPrograma(Estados* estados, int consolaFD);
bool findAndExitPCBnotExecuting(Estados* estados, int consolaFD);
void findAndExitPCBexecuting(Estados* estados, int consolaFD);
void informarPlanificador(int socketPlanificador, int accion, int pid);

#endif /* PCB_H_ */
