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

	void eliminar_fila(void *fila){
		free(fila);
	}

	tmp=(t_tabla*) tabla;

	list_destroy_and_destroy_elements(tmp->filas,eliminar_fila);

	free(tmp);
}

t_tabla* obtener_tabla(uint32_t pid){
	t_tabla *tabla_buscada;

	bool tabla_valida(void* tabla){
		return ((t_tabla*) tabla)->pid==pid;
	}

	return tabla_buscada=list_find(tablas_de_paginas,tabla_valida);
}

uint32_t obtener_numero_marco_tlb(uint32_t numero_pagina){
	t_fila_tlb *fila_buscada;

	bool fila_valida(void* fila){
		t_fila_tlb *tmp=(t_fila_tlb*) fila;

		return 	tmp->numero_pagina==numero_pagina &&
				tmp->pid==obtener_pid();
	}

	fila_buscada=list_find(tlb->filas,fila_valida);

	return fila_buscada ? fila_buscada->numero_marco : -1;
}

uint32_t obtener_numero_marco_tabla(uint32_t numero_pagina){

	t_tabla *tabla_buscada;
	t_fila_tabla *fila_buscada;

	bool fila_valida(void* fila){
		return ((t_fila_tabla*) fila)->numero_pagina==numero_pagina;
	}

	tabla_buscada=obtener_tabla(obtener_pid());
	if(tabla_buscada==NULL){
		logDebug("No se encontro la tabla");
		return -3;	//No existe la tabla del proceso, estamos hasta las manos
	}
	if(numero_pagina > tabla_buscada->tamanio){
		logDebug("Quiere entrar a una posicion que no le pertenece");
		return -2; //Quiere entrar a una posicion que no le pertenece
	}
	fila_buscada=list_find(tabla_buscada->filas,fila_valida);
	if(fila_buscada==NULL){
		logDebug("No se encontro la fila por lo que la pagina no esta en memoria");
		return -1;	//No se encontro la fila, por lo que la pagina no esta en memoria
	}

	logDebug("Numero de marco encontrado: %d", fila_buscada->numero_marco);
	return fila_buscada->numero_marco;
}

void cargar_dir_tabla(uint32_t numero_pagina, uint32_t numero_marco){

	t_tabla *tabla;
	t_fila_tabla *fila;

	bool fila_valida(void* fila){
		return ((t_fila_tabla*) fila)->numero_pagina==numero_pagina;
	}

	tabla=obtener_tabla(obtener_pid());	//La tabla a esta altura tiene que existir

	if(!(fila=list_find(tabla->filas,fila_valida))){
		//La fila puede no existir, la tabla se crea vacia
		fila=malloc(sizeof(t_fila_tabla));
		list_add(tabla->filas,(void*)fila);
	}

	fila->numero_pagina=numero_pagina;
	fila->numero_marco=numero_marco;
}

void borrar_dir_tablas(uint32_t numero_marco){

	void eliminar(void* fila){
		free(fila);
	}

	bool fila_valida(void* fila){
		return ((t_fila_tabla*)fila)->numero_marco == numero_marco;
	}

	void eliminar_fila(void* tabla){
		list_remove_and_destroy_by_condition(((t_tabla*)tabla)->filas, fila_valida, eliminar);
	}

	/* Itero todas las tablas porque no encuentro una forma copada de parar en la que tiene el marco.
	 * De todas formas no deberia traer problemas ya que un numero de marco solo puede pertenecer a un
	 * programa.
	 */
	list_iterate(tablas_de_paginas,eliminar_fila);

}

uint32_t obtener_marco_para_swap(){
	int i=0, marco_no_encontrado=1;

	while(marco_no_encontrado){
		pthread_mutex_lock(&activo_mutex);

		if(bitarray_test_bit(memoria_principal.activo,i))
			bitarray_clean_bit(memoria_principal.activo,i);
		else
			marco_no_encontrado=0;

		pthread_mutex_unlock(&activo_mutex);

		if(i == bitarray_get_max_bit(memoria_principal.activo)) i=0;
		else i++;
	}

	return i;
}

uint32_t swap_marco(uint32_t numero_pagina){
	int tamanio_pagina = config->size_pagina,
		pid = obtener_pid();
	uint32_t marco_elegido = obtener_marco_para_swap(),
			 dir_fisica = marco_elegido * tamanio_pagina;
	bool result;

	logDebug("Se realiza swap sobre marco %d para programa %d", marco_elegido, pid);

	pthread_mutex_lock(&modificacion_mutex);
	result = !bitarray_test_bit(memoria_principal.modificacion,marco_elegido);
	pthread_mutex_unlock(&modificacion_mutex);

	if(result)
		escribirPaginaSwap(pid, numero_pagina, tamanio_pagina,memoria_principal.memoria + dir_fisica);

	borrar_dir_tablas(marco_elegido);

	return marco_elegido;
}

uint32_t obtener_marco_libre(){

	int i=0;
	char bit_no_encontrado=1;

	while(i < config->cantidad_paginas && bit_no_encontrado){
		pthread_mutex_lock(&bitMap_mutex);

		if(bitarray_test_bit(memoria_principal.bitmap,i)==0) bit_no_encontrado = 0;
		else i++;

		pthread_mutex_unlock(&bitMap_mutex);
	}

	return bit_no_encontrado ? -1 : i;
}

void agregar_pagina_a_memoria(uint32_t pid, uint32_t numero_pagina, char* pagina){

	int marco_libre = obtener_marco_libre();
	uint32_t dir_fisica;

	if(marco_libre < 0) marco_libre = swap_marco(numero_pagina);	//Si es -1 hago swap, swap no deberia fallar nunca
	dir_fisica = marco_libre * config->size_pagina;

	logDebug("Se agrega pagina %d en marco %d",numero_pagina, marco_libre);
	memcpy(memoria_principal.memoria+dir_fisica,pagina,config->size_pagina);

	pthread_mutex_lock(&bitMap_mutex);
	bitarray_set_bit(memoria_principal.bitmap,marco_libre);
	pthread_mutex_unlock(&bitMap_mutex);

	cargar_dir_tabla(numero_pagina, marco_libre);
}

void copiar_pagina_a_tlb(uint32_t numero_pagina){
	//Todo copiar la pagina a la tlb para la proxima busqueda
}

void copiar_pagina_a_memoria(uint32_t numero_pagina){

	uint32_t pid = obtener_pid();
	char* pagina = leerPaginaSwap(pid, numero_pagina);

	if(!pagina) return;	//Todo ver de poner un error copado

	agregar_pagina_a_memoria(pid,numero_pagina,pagina);

	free(pagina);
}

uint32_t obtener_numero_marco(uint32_t numero_pagina){
	uint32_t nro_marco;

	if((nro_marco=obtener_numero_marco_tlb(numero_pagina)) == -1){
		if((nro_marco=obtener_numero_marco_tabla(numero_pagina)) == -1){
			copiar_pagina_a_memoria(numero_pagina);
			nro_marco=obtener_numero_marco_tabla(numero_pagina);
		}else copiar_pagina_a_tlb(numero_pagina);
	}

	return nro_marco;
}

//----------------------------------PUBLICO---------------------------------------

void crearMemoriaPrincipal(int cantidad_paginas, int size_pagina){

	char *bits_bitMap = malloc(sizeof(char)*cantidad_paginas),
		 *bits_modificacion = malloc(sizeof(char)*cantidad_paginas),
		 *bits_activo = malloc(sizeof(char)*cantidad_paginas);
	int i;

	logDebug("Creando memoria principal de tamanio %d\n", cantidad_paginas*size_pagina);

	for(i=0;i<cantidad_paginas;i++){
		bits_bitMap[i]=0;
		bits_modificacion[i]=0;
		bits_activo[i]=0;
	}

	memoria_principal.memoria = malloc(cantidad_paginas*size_pagina);
	memoria_principal.bitmap = bitarray_create(bits_bitMap,cantidad_paginas);
	memoria_principal.modificacion = bitarray_create(bits_modificacion,cantidad_paginas);
	memoria_principal.activo = bitarray_create(bits_activo,cantidad_paginas);

}

void crear_tabla_de_paginas(uint32_t pid, uint32_t cant_paginas){

	t_tabla *nueva_tabla = malloc(sizeof(t_tabla));

	nueva_tabla->pid = pid;
	nueva_tabla->tamanio = cant_paginas;
	nueva_tabla->filas = list_create();

	list_add(tablas_de_paginas,(void*) nueva_tabla);

}

void eliminar_tabla_de_paginas(uint32_t pid){

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

int obtener_contenido_memoria(char** contenido, uint32_t numero_pagina, uint32_t offset, uint32_t tamanio){

	uint32_t numero_marco=obtener_numero_marco(numero_pagina),
			 dir_fisica=numero_marco*config->size_pagina;
	memoria mem=memoria_principal.memoria+offset+dir_fisica;

	//Si no se pudo encontrar la direccion fisica devuelvo el codigo de error
	if(dir_fisica<0) return dir_fisica;

	//Pueden mandarme un puntero a NULL o un puntero a espacio ya reservado (actualmente solo pasa la primera)
	if(*contenido==NULL) *contenido=malloc(tamanio);

	memcpy(*contenido,mem,tamanio);

	pthread_mutex_lock(&activo_mutex);
	bitarray_set_bit(memoria_principal.activo,numero_marco);
	pthread_mutex_unlock(&activo_mutex);

	return tamanio;	//Si salio bien devuelvo el la cantidad de bytes que lei
}

int escribir_contenido_memoria(uint32_t numero_pagina, uint32_t offset, uint32_t tamanio, char* contenido){

	uint32_t numero_marco=obtener_numero_marco(numero_pagina),
			 dir_fisica=numero_marco * config->size_pagina;
	memoria mem=memoria_principal.memoria+offset+dir_fisica;

	//Si no se pudo encontrar la direccion fisica devuelvo el codigo de error
	if(dir_fisica<0) return dir_fisica;

	memcpy(mem,contenido,tamanio);

	pthread_mutex_lock(&modificacion_mutex);
	bitarray_set_bit(memoria_principal.modificacion,numero_marco);
	pthread_mutex_unlock(&modificacion_mutex);

	pthread_mutex_lock(&activo_mutex);
	bitarray_set_bit(memoria_principal.activo,numero_marco);
	pthread_mutex_unlock(&activo_mutex);

	return tamanio;	//Si salio bien devuelvo el la cantidad de bytes que escribi
}

void liberar_memoria(uint32_t pid){

	t_tabla *tabla_buscada;

	tabla_buscada=obtener_tabla(pid);

	void liberar_memoria_con_fila(void* fila){
		int nro_marco;

		nro_marco=((t_fila_tabla *) fila)->numero_marco;
		bitarray_clean_bit(memoria_principal.bitmap,nro_marco);
	}

	list_iterate(tabla_buscada->filas,liberar_memoria_con_fila);
}

void flush_tlb(){
	//Limpia el contenido de la TLB

	void eliminarFila(void* fila)
	{
		free(fila);
	}

	list_clean_and_destroy_elements(tlb->filas, eliminarFila);

}

void flush_memory(){
	//Marca todas las paginas como modificadas
	int i, tamanio;
	t_bitarray *bitMap_Modificacion = memoria_principal.modificacion;

	tamanio = bitarray_get_max_bit(bitMap_Modificacion);

	pthread_mutex_lock(&modificacion_mutex);
	for(i=0; i<tamanio; i++){

		bitarray_clean_bit(bitMap_Modificacion,i);
	}
	pthread_mutex_unlock(&modificacion_mutex);
}

void crearListaDeTablas(){

	tablas_de_paginas = list_create();
}


