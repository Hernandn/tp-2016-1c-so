/*
 * CPU.h
 *
 *  Created on: 19/4/2016
 *      Author: utnso
 */

#include "configuration.h"

#ifndef CPU_H_
#define CPU_H_

//codigos de operaciones de la UMC
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define INIT_PROGRAM 1
#define SOLICITAR_BYTES_PAGINA 2
#define ALMACENAR_BYTES_PAGINA 3
#define END_PROGRAM 4
#define HANDSHAKE 5
#define SWITCH_PROCESS 6
//---------------------
#define NEW_ANSISOP_PROGRAM 10

typedef struct arg_struct {
	Configuration* config;
    t_log* logger;
} arg_struct;

//prototipos de funciones
void conectarConUMC();
void conectarConNucleo();
Configuration* configurar(t_log* logger);


#endif /* CPU_H_ */
