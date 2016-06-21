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

t_bitarray* getBitMap(){
	return bitMap;
}

tableRow* getTablaDePaginas(){
	return tabla;
}

void nuevoPrograma(int frame, int pid, int cantPaginas){
	int i,j=0;
	for(i=frame; i<(cantPaginas+frame); i++){
		bitarray_set_bit(bitMap,i);
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


int obtener_primer_disp()
{
		//contado de espacio contiguo
		int  i;

		for(i=0; i<bitMap->size; i++){
			//a partir del primer 0 espacio disponible
			if(!bitarray_test_bit(bitMap,i)){
				return i;
			}
			else{
				return -1;
			}
		}

}

int ultimo_disponible(int primero)
{
	//contado de espacio contiguo
	int  i;
	int cont = 0;

	for(i=(primero); i<bitMap->size; i++){
		//a partir del primer 0 espacio disponible
		if(!bitarray_test_bit(bitMap,i)){
			cont++;
		}
	}
	return cont;
}

int cant_pag_frame (int frame)
{
	int cont, i;
	cont=0;
	for(i=0; i<config->cantidad_paginas;i++)
	{
		if (i==frame)
		{
			cont ++;
		}
	}
		return cont;
}

void moverframe(int frame_origen, int frame_destino)
{
	int i, cantPaginas;
	pagina pag;
	//setea el frame donde se va mover en 1
	bitarray_set_bit(bitMap,frame_destino);
	//trae la pagina del frame origen y la escribe en el frame destino
	pag = leerPaginaFromFrame(frame_origen);
	escribirPaginaEnFrame(frame_destino,pag);

	//de cada frame pasa las paginas necesarias al nuevo frame
	cantPaginas=cant_pag_frame(frame_origen);
	for(i=frame_destino; i<(cantPaginas+frame_destino); i++){

	tabla[frame_destino].page = tabla[frame_origen].page;
	tabla[frame_destino].pid = tabla[frame_origen].pid;

	tabla[frame_origen].page = -1;
	tabla[frame_origen].pid = -1;
	}
	bitarray_clean_bit(bitMap,frame_origen);

}

void compactacion (Configuration* config)
{
	while (1)
	{
		int i;
			//primer bloque disponible para empezar a compactar
			int primer_frame_disponible = obtener_primer_disp();
			//busco ultimo disponible empezando desde el primero disponible
			int ultimo_frame_disponible = ultimo_disponible(primer_frame_disponible);
			int j=0;
			i=primer_frame_disponible;
			//Voy acomodando desde el primero disponible hasta el ultimo de ese bloque todos los ocupados que
			//encuentro a partir de ahi
			while(i<= ultimo_frame_disponible)
			{
				if(bitarray_test_bit(bitMap,ultimo_frame_disponible+j))
				{
					moverframe(ultimo_frame_disponible+j, i);
					i++;
				}
				j++;

			}
	}
}
