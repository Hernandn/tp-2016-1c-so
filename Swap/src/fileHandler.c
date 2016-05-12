/*
 * fileHandler.c
 *
 *  Created on: 5/5/2016
 *      Author: utnso
 */

#include "fileHandler.h"

t_bitarray* bitMap;
tableRow* tabla;
FILE* file;
Configuration* config;

void inicializarSwap(Configuration* conf){
	config = conf;
	crearBitMap();
	crearTablaDePaginas();
	crearArchivoSwap();
}

void crearBitMap(){
	char* bits = malloc(sizeof(char)*config->cantidad_paginas);
	int i;
	for(i=0; i<config->cantidad_paginas; i++){
		bits[i]=0;
	}
	bitMap = bitarray_create(bits,config->cantidad_paginas);
}

int getFirstAvailableBlock(int cantPaginas){
	//total(espacio libre fragmentado), cont(contador de espacio contiguo)
	int total = 0, cont = 0, i;

	for(i=0; i<bitMap->size; i++){
		//si encuentro un bit en 0 (espacio libre) incremento, si no reseteo el contiguo
		if(!bitarray_test_bit(bitMap,i)){
			cont++;
			total++;
		} else {
			cont = 0;
		}
		//si encuentro un espacio contiguo de la cantidad de paginas pedidas, corto el for
		if(cont>=cantPaginas){
			i++;
			break;
		}
	}
	int retorna = -1;
	if(cont>=cantPaginas){//si corte porque hay un espacio, devuelvo la posicion a partir de la cual esta el espacio para empezar a escribir
		retorna = i-cantPaginas;
	} else if(total>=cantPaginas){//si hay lugar pero fragmentado, se devuelve -2 que es el codigo para saber si se puede defragmentar
		retorna = -2;
	} else {//devuelve -1 si no hay espacio
		retorna = -1;
	}
	return retorna;
}

void escribirPaginaEnFrame(int frame, pagina pag){
	fseek(file,frame*config->size_pagina,SEEK_SET);
	fwrite(pag,config->size_pagina,1,file);
	bitarray_set_bit(bitMap,frame);
}

void escribirPaginasEnFrame(int frame, pagina* paginas, int cantPaginas){
	int i;
	for(i=0; i<cantPaginas; i++){
		escribirPaginaEnFrame(frame+i,paginas[i]);
	}
}

int getFileSize(){
	fseek(file, 0, SEEK_END); // seek to end of file
	int size = ftell(file)/config->size_pagina; // get current file pointer
	fseek(file, 0, SEEK_SET);
	return size;
}


void cerrarArchivoSwap(){
	fclose(file);
}


pagina leerPaginaFromFrame(int frame){
	pagina pag = malloc(sizeof(char)*config->size_pagina);
	fseek(file,frame*config->size_pagina,SEEK_SET);
	fread(pag,config->size_pagina,1,file);
	return pag;
}

void crearArchivoSwap(){
	file = fopen(config->nombre_swap,"w+b");
}

void crearTablaDePaginas(){
	tabla = malloc(sizeof(tableRow)*config->cantidad_paginas);
	int i;
	for(i=0; i<config->cantidad_paginas; i++){
		tabla[i].pid = -1;
		tabla[i].page = -1;
	}
}

void destroyTabla(){
	free(tabla);
}

//esta funcion esta solo para ejemplificar como se maneja la pagina
//en este caso se crea una pagina de tamanio 8 bytes y se le asigna 2 enteros (uno del byte 0 al 3 y otro del 4 al 7)
void ejemploPagina(){
	pagina b = malloc(8);
	b = 7;
	printf("numero %d\n",b);
	b+=4;
	b = 9;
	printf("numero %d\n",b);
}

t_bitarray* getBitMap(){
	return bitMap;
}

tableRow* getTablaDePaginas(){
	return tabla;
}

void guardarPrograma(int frame, int pid, int cantPaginas, pagina* paginas){
	escribirPaginasEnFrame(frame,paginas,cantPaginas);
	int i,j=0;
	for(i=frame; i<(cantPaginas+frame); i++){
		tabla[i].pid = pid;
		tabla[i].page = j;
		j++;
	}
}

void eliminarPrograma(int pid){
	int i;
	for(i=0; i<config->cantidad_paginas; i++){
		//si encuentra un registro con el mismo processID, hace la baja logica y libera el espacio en el bitmap
		if(tabla[i].pid==pid){
			tabla[i].pid=-1;
			tabla[i].page=-1;
			bitarray_clean_bit(bitMap,i);
		}
	}
}

int buscarFramePorPagina(int pid, int pagina){
	int i;
	for(i=0; i<config->cantidad_paginas; i++){
		if(tabla[i].pid==pid && tabla[i].page==pagina){
			return i;//retorna la posicion en swap (frame)
		}
	}
	return -1;//retorna -1 si no encuentra la pagina del proceso en la tabla
}

pagina leerPaginaDeProceso(int pid, int paginaNro){
	int frame = buscarFramePorPagina(pid,paginaNro);
	if(frame>=0){
		return leerPaginaFromFrame(frame);
	} else {
		return NULL;
	}
}

void escribirPaginaDeProceso(int pid, int paginaNro, pagina pag){
	int frame = buscarFramePorPagina(pid,paginaNro);
	if(frame>=0){
		escribirPaginaEnFrame(frame,pag);
	}
}

