/*
 ============================================================================
 Name        : Swap.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Swap.h"

pagina escribir(char* cad, int size);

int main(void) {

	Configuration* config = configurar();

	//creo el log
	initLogMutex(config->log_file, config->log_program_name, config->log_print_console, log_level_from_string(config->log_level));

	inicializarSwap(config);
	t_bitarray* bitMap = getBitMap();
	tableRow* tabla = getTablaDePaginas();

	//Simulo la creacion de un programa
	int cantidadPaginas = 4;
	int pid = 2;
	pagina* paginas = malloc(sizeof(pagina)*cantidadPaginas);

	paginas[0] = escribir("a",config->size_pagina);
	paginas[1] = escribir("b",config->size_pagina);
	paginas[2] = escribir("c",config->size_pagina);
	paginas[3] = escribir("d",config->size_pagina);

	//simulo que estan ocupadas (fragmentacion)
	bitarray_set_bit(bitMap,3);
	bitarray_set_bit(bitMap,6);
	bitarray_set_bit(bitMap,9);

	int espacio = getFirstAvailableBlock(cantidadPaginas);
	printf("Primer espacio libre: %d\n",espacio);
	nuevoPrograma(espacio,pid,cantidadPaginas);
	//ya fueron reservadas las paginas, ahora escribo las paginas que forman el nuevo programa
	int i;
	for(i=0; i<cantidadPaginas; i++){
		escribirPaginaDeProceso(pid,i,paginas[i]);
	}

	pagina page3 = escribir("z",config->size_pagina);
	escribirPaginaDeProceso(pid,2,page3);

	//sobreescribo una pagina
	pagina page2 = leerPaginaDeProceso(pid,2);
	printf("\nPagina %d leida\n%s\n",2,page2);

	pagina page4 = leerPaginaDeProceso(pid,0);
	printf("\nPagina %d leida\n%s\n",0,page4);

	//imprimo en consola el contenido del bitmap
	printf("\nBitMap\n");
	printf("Frame:\t");

	for(i=0; i<bitMap->size; i++){
		printf("%d ",i);
	}
	printf("\nOcup:\t");
	for(i=0; i<bitMap->size; i++){
		printf("%d ",bitarray_test_bit(bitMap,i));
		if(i>=10)
			printf(" ");
		if(i>=100)
			printf(" ");
	}

	//imprimo en consola el contenido de la tabla de paginas
	printf("\n\nTabla de paginas\n");
	printf("Frame:\t");
	for(i=0; i<bitMap->size; i++){
		printf("%d   ",i);
	}
	printf("\nPID:\t");
	for(i=0; i<bitMap->size; i++){
		printf("%d  ",tabla[i].pid);
		if(tabla[i].pid>=0)
			printf(" ");
		if(i>=10)
			printf(" ");
		if(i>=100)
			printf(" ");
	}
	printf("\nPage:\t");
	for(i=0; i<bitMap->size; i++){
		printf("%d  ",tabla[i].page);
		if(tabla[i].page>=0)
			printf(" ");
		if(i>=10)
			printf(" ");
		if(i>=100)
			printf(" ");
	}
	printf("\n\n");
	for(i=0; i<cantidadPaginas; i++){
		free(paginas[i]);
	}
	free(paginas);

	handleUMCRequests(config);

	destroyTabla();
	cerrarArchivoSwap();

	logDestroy();

	return EXIT_SUCCESS;
}

//hace un relleno de pagina con la letra pasada en "cad" (para probar)
pagina escribir(char* cad, int size){
	pagina page = malloc(sizeof(char)*size);
	int j;
	page[0] = '\0';
	for(j=0; j<size; j++){
		strcat(page,cad);
	}
	page[j-1] = '\0';
	return page;
}
