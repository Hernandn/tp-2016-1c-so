/*
 * interfaz.c
 *
 *  Created on: 29/5/2016
 *      Author: utnso
 */

#include "interfaz.h"
#include "interfazSwap.h"

//----------------------------------PRIVADO---------------------------------------

static void deserializar_parametros(int cant_parametros, char* mensaje, ...){

		va_list valist;
		int i,
			offset=0,
			tamanio;
		void* parametro;

		va_start(valist, mensaje);

		for(i=0; i<cant_parametros; i++){

			tamanio=va_arg(valist, int);

			parametro=va_arg(valist, void*);
			memcpy(parametro,mensaje+offset,tamanio);

			offset +=tamanio;
		}

		va_end(valist);

}

//----------------------------------PUBLICO---------------------------------------

int inicializar_programa(char* mensaje_serializado){

	uint32_t pid, cant_paginas;
	char* contenido=NULL;
	int resultado;

	deserializar_parametros(3, mensaje_serializado, sizeof(uint32_t), (void*) &pid, sizeof(uint32_t), (void*) &cant_paginas, (int)cant_paginas, (void*) contenido);

	logDebug("Inicializando programa %d, cantidad paginas %d", pid, cant_paginas);

	if(!(resultado = comunicarSWAPNuevoPrograma(pid,cant_paginas,contenido))){
		crear_tabla_de_paginas(pid,cant_paginas);
		crear_tlb(pid,cant_paginas);
	}

	return resultado;
}

char* leer_pagina(char* mensaje_serializado){

	t_direccion_pagina dir;
	uint32_t tamanio;

	deserializar_parametros(3, mensaje_serializado, sizeof(t_direccion_pagina), (void*) &dir, sizeof(uint32_t), (void*) &tamanio);

	logDebug("Leyendo pagina %d, cantidad paginas %d", dir.direccion_logica, tamanio);

	return "";
}

int escribir_pagina(char* mensaje_serializado){

	t_direccion_pagina dir;
	uint32_t tamanio;
	char* contenido=NULL;

	deserializar_parametros(3, mensaje_serializado, sizeof(t_direccion_pagina), (void*) &dir, sizeof(uint32_t), (void*) &tamanio, sizeof(char*), (void*) contenido);

	logDebug("Escribiendo pagina %d, tamanio %d, contenido %s", dir.direccion_logica, tamanio, contenido);

	return 0;
}

int finalizar_programa(char* mensaje_serializado){

	uint32_t pid;

	deserializar_parametros(1, mensaje_serializado, sizeof(uint32_t), (void*) &pid);

	logDebug("Finalizando programa %d", pid);

	return 0;
}
