/*
 * memoria.c
 *
 *  Created on: 5/6/2016
 *      Author: utnso
 */

#include "memoria.h"

//----------------------------------PRIVADO---------------------------------------

static t_memoria_principal memoria_principal;
static t_list *tablas_de_paginas;
static t_tabla_tlb *tlb;

void destructor_tabla(void* tabla){
	t_tabla *tmp;

	tmp=(t_tabla*) tabla;

	if(tmp!=NULL){
		free(tmp->filas);
		free(tmp);
	}
}

uint32_t obtener_dir_fisica(uint32_t dir_logica){
	//Todo buscar la dir en la tlb
	return 0;
}

//----------------------------------PUBLICO---------------------------------------

void crearMemoriaPrincipal(int cantidad_paginas, int size_pagina){

	char* bits = malloc(sizeof(char)*cantidad_paginas);
	int i;

	logDebug("Creando memoria principal de tamanio %d\n", cantidad_paginas*size_pagina);

	for(i=0;i<cantidad_paginas;i++){
		bits[i]=0;
	}

	memoria_principal.memoria = malloc(cantidad_paginas*size_pagina);
	memoria_principal.bitmap = bitarray_create(bits,cantidad_paginas);

}

void crear_tabla_de_paginas(uint32_t pid, uint32_t cant_paginas){

	t_tabla *nueva_tabla = malloc(sizeof(t_tabla));

	nueva_tabla->pid = pid;
	nueva_tabla->tamanio = cant_paginas;
	nueva_tabla->filas = list_create();

	list_add(tablas_de_paginas,(void*) nueva_tabla);

}

void eliminar_tabla_de_paginas(uint32_t pid){

	logDebug("Eliminando tabla con pid %d", pid);

	bool tiene_igual_pid (void* elemento){
		return ((t_tabla*) elemento)->pid==pid;
	}

	list_remove_and_destroy_by_condition(tablas_de_paginas,tiene_igual_pid,destructor_tabla);

}

void crear_tlb(uint32_t tamanio){

	tlb = malloc(sizeof(t_tabla_tlb));

	tlb->tamanio=tamanio;
	tlb->filas=list_create();

	logDebug("Nueva TLB creada");

}

void elimina_tlb(t_tabla_tlb *tabla){

	void eliminar_fila(void *fila){
		free(fila);
	}

	list_destroy_and_destroy_elements(tabla->filas, eliminar_fila);

}

char* obtener_contenido_memoria(uint32_t dir_logica, uint32_t offset, uint32_t tamanio){
	//Todo busar en memoria el contenido
	return NULL;
}

int escribir_contenido_memoria(uint32_t dir_logica, uint32_t offset, uint32_t tamanio, char* contenido){
	//Todo escribir contenido en memoria
	return 0;
}
