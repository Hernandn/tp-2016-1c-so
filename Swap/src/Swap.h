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
#define SOLICITAR_BYTES_PAGINA_SWAP 20
#define ALMACENAR_BYTES_PAGINA_SWAP 21

void handleUMCRequests(Configuration* config);

#endif /* SWAP_H_ */
