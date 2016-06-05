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

int cant_paginas_afectadas(uint32_t offset, uint32_t tamanio){

	/* Haciendo el calculo (offset+tamanio)/tamanioPagina +1
	 * obtengo la cantidad de paginas afectadas por la
	 * operacion.
	 */

	return (offset+tamanio)/config->size_pagina +1;
}

int nro_de_pagina(uint32_t dir_fisica){
	return dir_fisica/config->size_pagina +1;
}

uint32_t obtener_dir_fisica_tlb(uint32_t dir_logica){
	t_fila_tlb *fila_buscada;

	bool fila_valida(void* fila){
		t_fila_tlb *tmp=(t_fila_tlb*) fila;

		return 	tmp->dir_logica==dir_logica &&
				tmp->pid==obtener_pid();
	}

	fila_buscada=list_find(tlb->filas,fila_valida);

	return fila_buscada ? fila_buscada->dir_fisica : -1;
}

uint32_t obtener_dir_fisisca_tabla(uint32_t dir_logica){

	t_tabla *tabla_buscada;
	t_fila_tabla *fila_buscada;

	bool tabla_valida(void* tabla){
		return ((t_tabla*) tabla)->pid==obtener_pid();
	}

	bool fila_valida(void* fila){
		return ((t_fila_tabla*) fila)->numero_pagina==dir_logica;
	}

	if((tabla_buscada=list_find(tablas_de_paginas,tabla_valida))) return -1;
	if((fila_buscada=list_find(tabla_buscada->filas,tabla_valida))) return -2; //Quiere entrar a una posicion que no le pertenece

	return 0;
}

void copiar_pagina_a_tlb(dir_logica){

}

uint32_t obtener_dir_fisica(uint32_t dir_logica){
	uint32_t dir_fisica;

	if((dir_fisica=obtener_dir_fisica_tlb(dir_logica)) == -1){
		if((dir_fisica=obtener_dir_fisisca_tabla(dir_logica)) == -1){
			//Todo aca tendria que traerme las paginas de swap
		}else copiar_pagina_a_tlb(dir_logica);
	}

	return dir_fisica;
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

int obtener_contenido_memoria(char* contenido, uint32_t dir_logica, uint32_t offset, uint32_t tamanio){

	uint32_t dir_fisica=obtener_dir_fisica(dir_logica);
	memoria mem=memoria_principal.memoria+offset+dir_fisica;

	//Si no se pudo encontrar la direccion fisica devuelvo el codigo de error
	if(dir_fisica<0) return dir_fisica;

	//Pueden mandarme un puntero a NULL o un puntero a espacio ya reservado (actualmente solo pasa la primera)
	if(contenido==NULL) contenido=malloc(sizeof(tamanio));

	memcpy(contenido,mem,tamanio);

	return 0;
}

int escribir_contenido_memoria(uint32_t dir_logica, uint32_t offset, uint32_t tamanio, char* contenido){

	uint32_t dir_fisica=obtener_dir_fisica(dir_logica);
	memoria mem=memoria_principal.memoria+offset+dir_fisica;
	int cant_paginas=cant_paginas_afectadas(offset,tamanio),
		i;

	//Si no se pudo encontrar la direccion fisica devuelvo el codigo de error
	if(dir_fisica<0) return dir_fisica;

	memcpy(mem,contenido,tamanio);

	for(i=0; i<cant_paginas; i++){
		bitarray_set_bit(memoria_principal.bitmap,nro_de_pagina(dir_fisica+i));
	}

	return 0;
}
