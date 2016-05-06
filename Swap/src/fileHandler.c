/*
 * fileHandler.c
 *
 *  Created on: 5/5/2016
 *      Author: utnso
 */

#include "fileHandler.h"

t_bitarray* crearBitMap(int cantidadPaginas){
	char* bits = malloc(sizeof(char)*cantidadPaginas);
	int i;
	for(i=0; i<cantidadPaginas; i++){
		bits[i]=0;
	}
	t_bitarray* bitArray = bitarray_create(bits,cantidadPaginas);
	return bitArray;
}

int getFirstAvailableBlock(FILE* file, t_bitarray* bitMap, int cantPaginas){
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

void escribirPaginaEnFrame(FILE* file, t_bitarray* bitMap, int frame, pagina pag, int sizePagina){
	fseek(file,frame*sizePagina,SEEK_SET);
	fwrite(pag,sizePagina,1,file);
	bitarray_set_bit(bitMap,frame);
}

void escribirPaginasEnFrame(FILE* file, t_bitarray* bitMap, int frame, pagina* pag, int cantPaginas, int sizePagina){
	int i;
	for(i=0; i<cantPaginas; i++){
		escribirPaginaEnFrame(file,bitMap,frame+i,pag[i],sizePagina);
	}
}

void setPaginasOcupadas(t_bitarray* bitMap, int offset, int cantidad){
	int i;
	for(i=offset; i<(offset+cantidad); i++){
		bitarray_set_bit(bitMap,i);
	}
}

int getFileSize(FILE* file, int sizePagina){
	fseek(file, 0, SEEK_END); // seek to end of file
	int size = ftell(file)/sizePagina; // get current file pointer
	fseek(file, 0, SEEK_SET);
	return size;
}


void cerrarArchivoSwap(FILE* file){
	fclose(file);
}


pagina leerPaginaFromFrame(FILE* file, int frame, int sizePagina){
	pagina pag = malloc(sizeof(char)*sizePagina);
	fseek(file,frame*sizePagina,SEEK_SET);
	fread(pag,sizePagina,1,file);
	return pag;
}

FILE* crearArchivoSwap(char* nombre){
	FILE* f = fopen(nombre,"w+b");
	return f;
}

tableRow* crearTablaDePaginas(int cantidadFrames){
	tableRow* table = malloc(sizeof(tableRow)*cantidadFrames);
	return table;
}

void destroyTabla(tableRow* table){
	free(table);
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
