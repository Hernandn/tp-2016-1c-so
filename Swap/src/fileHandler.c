/*
 * fileHandler.c
 *
 *  Created on: 5/5/2016
 *      Author: utnso
 */

#include "fileHandler.h"

t_bitarray* crearBitMap(int cantidadPaginas){
	char* bits = malloc(sizeof(char)*cantidadPaginas);
	t_bitarray* bitArray = bitarray_create(bits,cantidadPaginas);
	return bitArray;
}

void escribirPaginaEnFrame(FILE* file, int frame, pagina* pag, int sizePagina){
	fseek(file,frame*sizePagina,SEEK_SET);
	fwrite(pag,sizePagina,1,file);
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
	pagina pag;
	fseek(file,frame*sizePagina,SEEK_SET);
	fread(&pag,sizePagina,1,file);
	return pag;
}

FILE* crearArchivoSwap(char* nombre){
	FILE* f = fopen(nombre,"w+b");
	return f;
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
