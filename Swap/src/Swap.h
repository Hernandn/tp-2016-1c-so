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
int getProcessID_NuevoPrograma(char* str);
int getCantidadPaginas_NuevoPrograma(char* str);
pagina* getPaginas_NuevoPrograma(char* str, int cantPags, int size);
char* serializar_NuevoPrograma(uint32_t pid, uint32_t cantPags, pagina* paginas, int size_pagina);
int getProcessID_EscribirPagina(char* str);
int getNumeroPagina_EscribirPagina(char* str);
pagina getPagina_EscribirPagina(char* str, int size);
char* serializar_EscribirPagina(uint32_t pid, uint32_t numero_de_pagina, pagina pagina, int size_pagina);
int getProcessID_SolicitarPagina(char* str);
int getNumeroPagina_SolicitarPagina(char* str);
char* serializar_SolicitarPagina(uint32_t pid, uint32_t numero_de_pagina);

#endif /* SWAP_H_ */
