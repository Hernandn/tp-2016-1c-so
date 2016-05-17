/*
 * interfazSwap.h
 *
 *  Created on: 17/5/2016
 *      Author: utnso
 */

#ifndef INTERFAZSWAP_H_
#define INTERFAZSWAP_H_

#include "configuration.h"
#include "funciones.h"
#include <commons/string.h>

char* serializar_NuevoPrograma(uint32_t pid, uint32_t cantPags, pagina* paginas, int size_pagina);
int getLong_NuevoPrograma(int cantPags, int size_pagina);
char* serializar_EscribirPagina(uint32_t pid, uint32_t numero_de_pagina, pagina pagina, int size_pagina);
int getLong_EscribirPagina(int size_pagina);
char* serializar_SolicitarPagina(uint32_t pid, uint32_t numero_de_pagina);
int getLong_SolicitarPagina();
char* serializar_EliminarPrograma(uint32_t pid);
int getLong_EliminarPrograma();
pagina llenarPagina(char* cad, int size);

#endif /* INTERFAZSWAP_H_ */
