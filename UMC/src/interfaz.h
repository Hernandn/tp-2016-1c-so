/*
 * interfaz.h
 *
 *  Created on: 29/5/2016
 *      Author: utnso
 */

#ifndef INTERFAZ_H_
#define INTERFAZ_H_

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <mllibs/log/logger.h>

int inicializar_programa(char*);
char* leer_pagina(char*);
int escribir_pagina(char*);
int finalizar_programa(char*);

typedef union direccion_pagina {
	uint32_t direccion_logica;
	uint16_t direccion_fisica[2];
} t_direccion_pagina;

#endif /* INTERFAZ_H_ */
