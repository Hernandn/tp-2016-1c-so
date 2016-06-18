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
	uint32_t size_programa;
	char *contenido=NULL,		//Puntero donde se deserializa el codigo del programa
		 *tmp=NULL;				//Puntero a buffer multiplo de tamanio de pagina para compatibilizar el programa
	int resultado, i, tamanio_pagina = config->size_pagina;

	logDebug("----------------------Comienza inicializacion de programa----------------------\n");

	deserializar_parametros(3, mensaje_serializado, sizeof(uint32_t), (void*) &pid, sizeof(uint32_t), (void*) &cant_paginas, sizeof(uint32_t), (void*) &size_programa);
	contenido = malloc(sizeof(char)*size_programa);
	memcpy(contenido,mensaje_serializado+sizeof(uint32_t)*3,size_programa);

	logDebug("Inicializando programa %d, cantidad paginas %d", pid, cant_paginas);

	if((resultado = comunicarSWAPNuevoPrograma(pid,cant_paginas)) == 0){

		//Guardo el codigo en un buffer de tamanio multipo de tamanio de pagina
		tmp = malloc(tamanio_pagina * cant_paginas);
		memcpy(tmp,contenido,strlen(contenido));

		//Si hay espacio mando las paginas una por una y creo la tabla de paginas
		for(i=0; i<cant_paginas; i++){
			escribirPaginaSwap(pid,i,tamanio_pagina,tmp + (i * tamanio_pagina));
		}
		crear_tabla_de_paginas(pid,cant_paginas);
	}

	free(tmp);
	free(contenido);

	logDebug("----------------------Finaliza inicializacion de programa----------------------\n");

	return resultado;
}

int leer_pagina(char* mensaje_serializado, char** contenido){

	uint32_t dir, offset, tamanio;

	logDebug("----------------------Comienza lectura de pagina----------------------\n");

	deserializar_parametros(3, mensaje_serializado, sizeof(uint32_t), (void*) &dir, sizeof(uint32_t), (void*) &offset, sizeof(uint32_t), (void*) &tamanio);

	logDebug("Leyendo pagina %d, cantidad paginas %d", dir, tamanio);

	logDebug("----------------------Finaliza lectura de pagina----------------------\n");

	return obtener_contenido_memoria(contenido, dir, offset, tamanio);
}

int escribir_pagina(char* mensaje_serializado){

	uint32_t dir, offset, tamanio;
	char* contenido=NULL;

	logDebug("----------------------Comienza escritura de pagina----------------------\n");

	deserializar_parametros(3, mensaje_serializado, sizeof(uint32_t), (void*) &dir, sizeof(uint32_t), (void*) &offset, sizeof(uint32_t), (void*) &tamanio);
	contenido = malloc(sizeof(char)*tamanio);
	memcpy(contenido,mensaje_serializado+sizeof(uint32_t)*3,tamanio);

	logDebug("Escribiendo pagina %d, tamanio %d, contenido %s", dir, tamanio, contenido);

	logDebug("----------------------Finaliza escritura de pagina----------------------\n");
	return escribir_contenido_memoria(dir, offset, tamanio, contenido);

}

int finalizar_programa(char* mensaje_serializado){

	uint32_t pid;

	logDebug("----------------------Comienza finalizacion de programa----------------------\n");

	deserializar_parametros(1, mensaje_serializado, sizeof(uint32_t), (void*) &pid);

	logDebug("Finalizando programa %d", pid);

	liberar_memoria(pid);
	eliminar_tabla_de_paginas(pid);
	finalizarProgramaSwap(pid);

	logDebug("----------------------Finaliza finalizacion de programa----------------------\n");

	return 0;
}

void nuevo_pid(char* mensaje_serializado){

	uint32_t pid;

	deserializar_parametros(1, mensaje_serializado, sizeof(uint32_t), (void*) &pid);
	logDebug("Se recivio un nuevo pid %d",pid);

	setear_pid(pid);

}
