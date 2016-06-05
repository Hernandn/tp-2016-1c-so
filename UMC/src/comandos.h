/*
 * comandos.h
 *
 *  Created on: 5/6/2016
 *      Author: utnso
 */

#ifndef COMANDOS_H_
#define COMANDOS_H_

#include <pthread.h>
#include "funciones.h"
#include "UMC.h"

pthread_mutex_t continua_mutex;

int continua;

void retardo (int segundos);
void handleComandos();
void intepretarComando(char*);
void error_comando(char*);
void flush_memory();
void flush_tlb();
void retardo (int);
void dump ();
int parsear_comando(char*, char **);
void fin_programa();
void print_retardo();
void limpiar_pantalla();

#endif /* COMANDOS_H_ */
