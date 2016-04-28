/*
 * PCB.h
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#ifndef PCB_H_
#define PCB_H_

#include <commons/collections/queue.h>

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
	int programCounter;	//contador de programa
	int pagesQty;		//cantidad de paginas de codigo ANSISOP
	ParCodigo* codeIndex;		//indice de codigo
	int* tagIndex;		//indice de etiquetas
	int* stackIndex;		//indice del stack
} PCB;


/*
 * Estructura con las colas de Estados
 */
typedef struct Estados {
	t_queue* new;
	t_queue* ready;
	t_queue* execute;
	t_queue* block;
	t_queue* exit;
} Estados;



//funciones
PCB* buildNewPCB();
void destroyPCB(PCB* self);
int getNextPID();
void inicializarEstados(Estados* estados);
void sendToNEW(PCB* pcb, Estados* estados);
PCB* getNextFromNEW(Estados* estados);
PCB* getNextFromREADY(Estados* estados);
void sendToREADY(PCB* pcb, Estados* estados);
void sendToEXEC(PCB* pcb, Estados* estados);
void sendToBLOCK(PCB* pcb, Estados* estados);
void sendToEXIT(PCB* pcb, Estados* estados);

#endif /* PCB_H_ */