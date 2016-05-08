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

	int cantidadPaginas = 4;

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
	printf("\nPrimer espacio libre: %d\n",espacio);
	escribirPaginasEnFrame(espacio,paginas,cantidadPaginas,config->size_pagina);

	pagina pg = escribir("z",config->size_pagina);
	escribirPaginaEnFrame(8,pg,config->size_pagina);

	pagina* pg1 = malloc(sizeof(pagina));
	pg1[0] = escribir("g",config->size_pagina);
	escribirPaginasEnFrame(2,pg1,1,config->size_pagina);

	pagina page2 = leerPaginaFromFrame(2,config->size_pagina);
	printf("\nPagina leida\n%s\n",page2);


	printf("\nPosiciones bitmap\n");
	printf("Pos:\t");
	int i;
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

	for(i=0; i<cantidadPaginas; i++){
		free(paginas[i]);
	}
	free(paginas);
	destroyTabla();
	cerrarArchivoSwap();

	//handleUMCRequests(config);

	logDestroy();

	return EXIT_SUCCESS;
}

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
