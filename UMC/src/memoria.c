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

t_tabla* obtener_tabla(pid){
	t_tabla *tabla_buscada;

	bool tabla_valida(void* tabla){
		return ((t_tabla*) tabla)->pid==pid;
	}

	return tabla_buscada=list_find(tablas_de_paginas,tabla_valida);
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

	bool fila_valida(void* fila){
		return ((t_fila_tabla*) fila)->numero_pagina==dir_logica;
	}

	tabla_buscada=obtener_tabla(obtener_pid());
	if(tabla_buscada==NULL){
		return -3;	//No existe la tabla del proceso, estamos hasta las manos
	}
	if(dir_logica > tabla_buscada->tamanio){
		return -2; //Quiere entrar a una posicion que no le pertenece
	}
	fila_buscada=list_find(tabla_buscada->filas,fila_valida);
	if(fila_buscada==NULL){
		return -1;	//No se encontro la fila, por lo que la pagina no esta en memoria
	}

	return fila_buscada->numero_marco;
}

void cargar_dir_tabla(uint32_t dir_logica, uint32_t dir_fisica){

	t_tabla *tabla;
	t_fila_tabla *fila;

	bool fila_valida(void* fila){
		return ((t_fila_tabla*) fila)->numero_pagina==dir_logica;
	}
	//crearListaDeTablas(); no se por que estaba esto aca pero lo comento porque se esta volviendo a crear la lista y se borra lo anterior
	tabla=obtener_tabla(obtener_pid());	//La tabla a esta altura tiene que existir

	if(!(fila=list_find(tabla->filas,fila_valida))){
		//La fila puede no existir, la tabla se crea vacia
		fila=malloc(sizeof(t_fila_tabla));
		list_add(tabla->filas,(void*)fila);
	}

	fila->modificacion=0;
	fila->numero_pagina=dir_logica;
	fila->numero_marco=dir_fisica;
}

uint32_t obtener_marco_libre(){

	int i=0;
	char bit_no_encontrado=1;

	while(i < config->cantidad_paginas && bit_no_encontrado){
		if(bitarray_test_bit(memoria_principal.bitmap,i)==0) bit_no_encontrado = 0;
		else i++;
	}

	return bit_no_encontrado ? -1 : i;
}

void agregar_pagina_a_memoria(uint32_t pid, uint32_t dir_logica, char* pagina){

	int marco_libre = obtener_marco_libre();
	uint32_t dir_fisica = marco_libre * config->size_pagina;	//Todo No contemplo que no encuentre marco libre

	memcpy(memoria_principal.memoria+dir_fisica,pagina,config->size_pagina);

	bitarray_set_bit(memoria_principal.bitmap,marco_libre);

	cargar_dir_tabla(dir_logica, dir_fisica);
}

void copiar_pagina_a_tlb(uint32_t dir_logica){
	//Todo copiar la pagina a la tlb para la proxima busqueda
}

void copiar_pagina_a_memoria(uint32_t dir_logica){

	uint32_t pid = obtener_pid();
	char* pagina = leerPaginaSwap(pid, dir_logica);

	if(!pagina) return;	//Todo ver de poner un error copado

	agregar_pagina_a_memoria(pid,dir_logica,pagina);

	free(pagina);
}

uint32_t obtener_dir_fisica(uint32_t dir_logica){
	uint32_t dir_fisica;

	if((dir_fisica=obtener_dir_fisica_tlb(dir_logica)) == -1){
		if((dir_fisica=obtener_dir_fisisca_tabla(dir_logica)) == -1){
			copiar_pagina_a_memoria(dir_logica);
			dir_fisica=obtener_dir_fisisca_tabla(dir_logica);
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

int obtener_contenido_memoria(char** contenido, uint32_t dir_logica, uint32_t offset, uint32_t tamanio){

	uint32_t dir_fisica=obtener_dir_fisica(dir_logica);
	memoria mem=memoria_principal.memoria+offset+dir_fisica;

	//Si no se pudo encontrar la direccion fisica devuelvo el codigo de error
	if(dir_fisica<0) return dir_fisica;

	//Pueden mandarme un puntero a NULL o un puntero a espacio ya reservado (actualmente solo pasa la primera)
	if(*contenido==NULL) *contenido=malloc(tamanio);

	memcpy(*contenido,mem,tamanio);

	return tamanio;	//Si salio bien devuelvo el la cantidad de bytes que lei
}

int escribir_contenido_memoria(uint32_t dir_logica, uint32_t offset, uint32_t tamanio, char* contenido){

	uint32_t dir_fisica=obtener_dir_fisica(dir_logica);
	memoria mem=memoria_principal.memoria+offset+dir_fisica;

	//Si no se pudo encontrar la direccion fisica devuelvo el codigo de error
	if(dir_fisica<0) return dir_fisica;

	memcpy(mem,contenido,tamanio);

	return tamanio;	//Si salio bien devuelvo el la cantidad de bytes que escribi
}

void liberar_memoria(pid){

	t_tabla *tabla_buscada;

	tabla_buscada=obtener_tabla(pid);

	void liberar_memoria_con_fila(void* fila){
		int nro_marco;

		nro_marco=((t_fila_tabla *) fila)->numero_marco / config->size_pagina;
		bitarray_clean_bit(memoria_principal.bitmap,nro_marco);
	}

	list_iterate(tabla_buscada->filas,liberar_memoria_con_fila);
}

void flush_tlb()
{
	//Limpia el contenido de la TLB

	void eliminarFila(void* fila)
	{
		free(fila);
	}

	list_clean_and_destroy_elements(tlb->filas, eliminarFila);

}

void flush_memory()
{
	//Marca todas las paginas como modificadas

	void modifBit(void * fila)
	{
		logDebug("adentro de fila");
		((t_fila_tabla*)fila)->modificacion = 1;
	}
	void modifTabla(void * tabla)
	{
		logDebug("adentro de tabla");
		list_iterate(((t_tabla*) tabla)->filas, modifBit);
	}
	logDebug("HOLOOOOOOOOOOOOOOOOOOOOO");
	list_iterate(tablas_de_paginas,modifTabla);

}

void crearListaDeTablas()
{
	tablas_de_paginas = list_create();

}


