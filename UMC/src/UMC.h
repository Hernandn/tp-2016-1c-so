/*
 * UMC.h
 *
 *  Created on: 19/4/2016
 *      Author: utnso
 */

#include <commons/log.h>
#include <commons/config.h>
#include "configuration.h"

#ifndef UMC_H_
#define UMC_H_

//codigos de operaciones de la UMC
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define INIT_PROGRAM 1
#define SOLICITAR_BYTES_PAGINA 2
#define ALMACENAR_BYTES_PAGINA 3
#define END_PROGRAM 4
#define HANDSHAKE 5
#define SWITCH_PROCESS 6
//---------------------

#define MAX_CLIENTES 10 //cantidad maxima de conexiones por socket (CPUs)

//prototipos de funciones
void handleClients(Configuration* config, t_log* logger);
void comunicarSWAP(int socketSWAP, int accion);
Configuration* configurar(t_log* logger);

#endif /* UMC_H_ */
