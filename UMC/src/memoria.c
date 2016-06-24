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
		list_add_in_index(tabla->filas,tabla->puntero,(void*)fila);
		tabla->puntero++;
		tabla->puntero %= tabla->filas->elements_count;
	}

	fila->numero_pagina=numero_pagina;
	fila->numero_marco=numero_marco;
	fila->accedido=1;
	fila->modificado=0;
}

void borrar_dir_tabla(t_tabla *tabla_prc, uint32_t numero_marco){

	void eliminar(void* fila){
		free(fila);
	}

	bool fila_valida(void* fila){
		return ((t_fila_tabla*)fila)->numero_marco == numero_marco;
	}


	list_remove_and_destroy_by_condition(tabla_prc->filas, fila_valida, eliminar);
}

uint32_t swap_marco(uint32_t numero_pagina, t_tabla *tabla_prc){
	int tamanio_pagina = config->size_pagina,
		pid = obtener_pid();
	logDebug("Limite marcos x proceso alcanzado PID: %d, ejecutando algoritmo de reemplazo", pid);

	t_list* filas = tabla_prc->filas;
	uint32_t marco_elegido;
	t_fila_tabla* fila;
	bool marco_encontrado = false;

	if(config->algoritmo==CLOCK){

		while(!marco_encontrado){
			fila = list_get(filas,tabla_prc->puntero);
			if(fila->accedido){
				fila->accedido = 0;
				tabla_prc->puntero++;
				tabla_prc->puntero %= filas->elements_count;
			} else {
				marco_encontrado = true;
			}
		}
		marco_elegido = fila->numero_marco;
	} else {
		int contador_vuelta = 0;
		while(!marco_encontrado){
			//Paso 1
			while(!marco_encontrado && contador_vuelta < filas->elements_count){
				fila = list_get(filas,tabla_prc->puntero);
				if(!fila->accedido && !fila->modificado){
					marco_encontrado = true;
				} else {
					tabla_prc->puntero++;
					tabla_prc->puntero %= filas->elements_count;
				}
				contador_vuelta++;
			}
			contador_vuelta = 0;
			//Paso 2
			while(!marco_encontrado && contador_vuelta < filas->elements_count){
				fila = list_get(filas,tabla_prc->puntero);
				if(!fila->accedido && fila->modificado){
					marco_encontrado = true;
				} else {
					fila->accedido = 0;
					tabla_prc->puntero++;
					tabla_prc->puntero %= filas->elements_count;
				}
				contador_vuelta++;
			}
			contador_vuelta = 0;
		}
		marco_elegido = fila->numero_marco;
	}

	uint32_t dir_fisica = marco_elegido * tamanio_pagina;

	if(fila->modificado){
		logDebug("Se realiza swap de pagina para PID: %d", fila->numero_pagina, pid);
		escribirPaginaSwap(pid, fila->numero_pagina, tamanio_pagina,memoria_principal.memoria + dir_fisica);
	} else {
		logDebug("No fue necesario realizar swap de pagina %d para PID: %d", fila->numero_pagina, pid);
	}

	borrar_dir_tabla(tabla_prc,marco_elegido);

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

int agregar_pagina_a_memoria(uint32_t pid, uint32_t numero_pagina, char* pagina){

	int marco_libre;
	t_tabla *tabla_buscada = obtener_tabla(obtener_pid());
	if(tabla_buscada->filas->elements_count < config->marcos_x_proc){
		marco_libre	= obtener_marco_libre();
	} else {
		marco_libre = -1;
	}

	uint32_t dir_fisica;

	if(marco_libre < 0){
		if(list_is_empty(tabla_buscada->filas)){
			logDebug("No hay marcos libres ni tiene marcos para hacer swap [PID: %d]",pid);
			return -1;//Si no se encontro marco libre y el proceso no tiene ningun marco asignado para swapear
		}
		marco_libre = swap_marco(numero_pagina,tabla_buscada);	//Si es -1 hago swap, swap no deberia fallar nunca
	}
	dir_fisica = marco_libre * config->size_pagina;

	logDebug("Se agrega pagina %d en marco %d",numero_pagina, marco_libre);
	memcpy(memoria_principal.memoria+dir_fisica,pagina,config->size_pagina);

	pthread_mutex_lock(&bitMap_mutex);
	bitarray_set_bit(memoria_principal.bitmap,marco_libre);
	pthread_mutex_unlock(&bitMap_mutex);

	cargar_dir_tabla(numero_pagina, marco_libre);
	return marco_libre;
}

void copiar_pagina_a_tlb(uint32_t numero_pagina, uint32_t numero_marco){
	//Todo copiar la pagina a la tlb para la proxima busqueda
}

int copiar_pagina_a_memoria(uint32_t numero_pagina){

	uint32_t pid = obtener_pid();
	char* pagina = leerPaginaSwap(pid, numero_pagina);

	if(!pagina) return -1;	//Todo ver de poner un error copado

	int result = agregar_pagina_a_memoria(pid,numero_pagina,pagina);

	free(pagina);
	return result;
}

int obtener_numero_marco(uint32_t numero_pagina){
	uint32_t nro_marco;

	if((nro_marco=obtener_numero_marco_tlb(numero_pagina)) == -1){
		if((nro_marco=obtener_numero_marco_tabla(numero_pagina)) == -1){
			int result = copiar_pagina_a_memoria(numero_pagina);
			if(result<0){
				return -1;
			}
			nro_marco=obtener_numero_marco_tabla(numero_pagina);
			copiar_pagina_a_tlb(numero_pagina,nro_marco);
		} else {
			copiar_pagina_a_tlb(numero_pagina,nro_marco);
		}
	}

	return nro_marco;
}

t_fila_tabla* get_fila_tabla(uint32_t numero_pagina){
	t_tabla *tabla_buscada;
	t_fila_tabla *fila_buscada;

	bool fila_valida(void* fila){
		return ((t_fila_tabla*) fila)->numero_pagina==numero_pagina;
	}

	tabla_buscada=obtener_tabla(obtener_pid());
	fila_buscada=list_find(tabla_buscada->filas,fila_valida);
	return fila_buscada;
}

void marcar_pagina_accedida(uint32_t numero_pagina){
	t_fila_tabla *fila = get_fila_tabla(numero_pagina);
	fila->accedido=1;
}

void marcar_pagina_modificada(uint32_t numero_pagina){
	t_fila_tabla *fila = get_fila_tabla(numero_pagina);
	fila->modificado=1;
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
	nueva_tabla->tamanio = cant_paginas;//TODO esto me parece que no sirve
	nueva_tabla->filas = list_create();
	nueva_tabla->puntero = 0;

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

	int numero_marco=obtener_numero_marco(numero_pagina),
			 dir_fisica=numero_marco*config->size_pagina;
	memoria mem=memoria_principal.memoria+offset+dir_fisica;

	//Si no se pudo encontrar la direccion fisica devuelvo el codigo de error
	if(dir_fisica<0) return dir_fisica;

	//Pueden mandarme un puntero a NULL o un puntero a espacio ya reservado (actualmente solo pasa la primera)
	if(*contenido==NULL) *contenido=malloc(tamanio);

	memcpy(*contenido,mem,tamanio);

	marcar_pagina_accedida(numero_pagina);

	return tamanio;	//Si salio bien devuelvo el la cantidad de bytes que lei
}

int escribir_contenido_memoria(uint32_t numero_pagina, uint32_t offset, uint32_t tamanio, char* contenido){

	uint32_t numero_marco=obtener_numero_marco(numero_pagina),
			 dir_fisica=numero_marco * config->size_pagina;
	memoria mem=memoria_principal.memoria+offset+dir_fisica;

	//Si no se pudo encontrar la direccion fisica devuelvo el codigo de error
	if(dir_fisica<0) return dir_fisica;

	memcpy(mem,contenido,tamanio);

	marcar_pagina_accedida(numero_pagina);
	marcar_pagina_modificada(numero_pagina);

	return tamanio;	//Si salio bien devuelvo el la cantidad de bytes que escribi
}

int hayMarcosLibres(){
	return obtener_marco_libre();
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

void mostrar_tablas_pag ()	//TODO hice una impresion asi rapida para probar nomas, falta mejorarla
{
	void imprimirFilaMarcos(void* aux){
		t_fila_tabla* fila = (t_fila_tabla *) aux;
		printf("%d, ",fila->numero_marco);
	}
	void imprimirFilaPaginas(void* aux){
		t_fila_tabla* fila = (t_fila_tabla *) aux;
		printf("%d, ",fila->numero_pagina);
	}

	void imprimirTabla(void* aux){
		t_tabla* tabla = (t_tabla *) aux;
		printf("\nTabla de paginas PID: %d", tabla->pid);
		printf("\nMarcos: ");
		list_iterate(tabla->filas,imprimirFilaMarcos);
		printf("\nPaginas: ");
		list_iterate(tabla->filas,imprimirFilaPaginas);
	}

	list_iterate(tablas_de_paginas,imprimirTabla);

}

void mostrar_pag(int pid)
{

}

/*dump: Este comando generará un reporte en pantalla y en un archivo en disco del estado actual de:
Estructuras de memoria: Tablas de páginas de todos los procesos o de un proceso en particular.
Contenido de memoria: Datos almacenados en la memoria de todos los procesos o de un proceso en particular.
*/

void dump (int pid){

	FILE * reporte;
	reporte = fopen ("reporte.txt", "w+");
	//si el pid es 0 imprimo todos los procesos
	if (pid == 0)
	{
		mostrar_tablas_pag();
	}

	else
	{
		mostrar_pag(pid);
	}

}


