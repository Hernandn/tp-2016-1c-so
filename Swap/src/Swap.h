/*
 * Swap.h
 *
 *  Created on: 22/04/2016
 *      Author: hernan
 */

#ifndef SWAP_H_
#define SWAP_H_

#include "configuration.h"
#include "fileHandler.h"

//operaciones SWAP
#define SOLICITAR_PAGINA_SWAP 20
#define ALMACENAR_PAGINA_SWAP 21
#define ALMACENAR_NUEVO_PROGRAMA_SWAP 22
#define ELIMINAR_PROGRAMA_SWAP 23

void handleUMCRequests(Configuration* config);
void analizarMensaje(Package* package, int socketUMC, Configuration* config);

#endif /* SWAP_H_ */
