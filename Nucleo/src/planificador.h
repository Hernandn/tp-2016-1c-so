/*
 * planificador.h
 *
 *  Created on: 28/4/2016
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#define MAX_CONEXIONES 5	//en realidad son 2 por ahora (1 para el thread de consolas y 1 para el de CPUs)
#define PLANIFICADOR_IP "127.0.0.1"
#define PLANIFICADOR_PORT 6999

//codigos operaciones planificador
#define CPU_LIBRE 90
#define PROGRAM_READY 91

void planificar(void* arguments);
void atenderProcesos(Estados* estados, t_list* listaCPUs);
bool hayProcesosEnREADY(Estados* estados);
CPU* getCPUlibre(t_list* listaCPUs);
void inicializarSockets(int* sockets);

#endif /* PLANIFICADOR_H_ */
