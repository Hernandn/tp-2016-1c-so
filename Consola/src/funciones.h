/*
 * funciones.h
 *
 *  Created on: 5/5/2016
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include <mllibs/sockets/client.h>
#include <mllibs/sockets/package.h>
#include "configuration.h"

//codigos de operaciones de la Consola
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define NEW_ANSISOP_PROGRAM 10
#define ANSISOP_PROGRAM 11
#define HANDSHAKE 12
//---------------------

//prototipos de funciones
void handshake(Package*, int);
void comunicacionConNucleo(Configuration*, char*);
void iniciarProgramaAnsisop(Package* , int, char*);
char* obtener_programa(char*);


#endif /* FUNCIONES_H_ */
