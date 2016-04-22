/*
 * Nucleo.h
 *
 *  Created on: 21/04/2016
 *      Author: hernan
 */

#include <mllibs/sockets/package.h>

#ifndef NUCLEO_H_
#define NUCLEO_H_

#define MAX_CPUS 10	//cantidad de CPUs que se pueden conectar
#define MAX_CONSOLAS 10	//cantidad de Consolas que se pueden conectar

//codigos de operaciones de la Consola
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define NEW_ANSISOP_PROGRAM 10
#define ANSISOP_PROGRAM 11
#define HANDSHAKE 12
//---------------------

typedef struct arg_struct {
    int cpuSockets[MAX_CPUS];
    int consolaSockets[MAX_CONSOLAS];
    int socketServer;
} arg_struct;

//prototipos de funciones
void handleClients();
void handleConsolas();
void handleCPUs();
int elegirRandomCPU(int cpuSockets[]);
void comunicarCPU(int cpuSockets[]);
void enviarMensajeCPU(Package* package,int socket);

#endif /* NUCLEO_H_ */
