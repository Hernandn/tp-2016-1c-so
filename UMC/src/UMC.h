/*
 * UMC.h
 *
 *  Created on: 19/4/2016
 *      Author: utnso
 */

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
void handleClients();

#endif /* UMC_H_ */
