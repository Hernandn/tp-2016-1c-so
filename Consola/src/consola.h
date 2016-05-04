/*
 * consola.h
 *
 *  Created on: 21/4/2016
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <mllibs/sockets/package.h>
#include <mllibs/log/logger.h>
#include "configuration.h"

//codigos de operaciones de la Consola
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define NEW_ANSISOP_PROGRAM 10
#define ANSISOP_PROGRAM 11
#define HANDSHAKE 12
//---------------------

//prototipos de funciones
void handshake(Package* ,int);
void comunicacionConNucleo(Configuration*);
void iniciarProgramaAnsisop(Package* ,int);

#endif /* CONSOLA_H_ */
