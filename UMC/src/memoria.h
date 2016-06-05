/*
 * memoria.h
 *
 *  Created on: 5/6/2016
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <mllibs/log/logger.h>
#include "configuration.h"
#include "funciones.h"


typedef char* pagina;
typedef char* memoria;

typedef struct memoria_principal{
	memoria memoria;
	t_bitarray* bitmap;
} t_memoria_principal;

typedef struct fila_tabla{
	uint16_t numero_pagina;
	uint16_t numero_marco;
	char modificacion;
} t_fila_tabla;

typedef struct tabla{
	t_list* filas;
	uint32_t pid;
	uint32_t tamanio;
} t_tabla;

typedef struct fila_tlb{
	uint32_t dir_logica;
	uint32_t dir_fisica;
	uint32_t pid;
} t_fila_tlb;

typedef struct tabla_tlb{
	uint32_t tamanio;
	t_list* filas;
} t_tabla_tlb;

void crearMemoriaPrincipal(int, int);
void crear_tabla_de_paginas(uint32_t,uint32_t);
void crear_tlb(uint32_t tamanio);
void eliminar_tabla_de_paginas(uint32_t);
void eliminar_tabla(t_tabla*, t_tabla**, int);
int obtener_contenido_memoria(char*, uint32_t, uint32_t, uint32_t);
int escribir_contenido_memoria(uint32_t, uint32_t, uint32_t, char*);

#endif /* MEMORIA_H_ */
