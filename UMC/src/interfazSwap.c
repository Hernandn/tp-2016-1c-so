/*
 * interfazSwap.c
 *
 *  Created on: 17/5/2016
 *      Author: utnso
 */

#include "interfazSwap.h"

char* serializar_NuevoPrograma(uint32_t pid, uint32_t cantPags){
	//mensaje: pid + cantPags
	char *serializedPackage = malloc(sizeof(uint32_t)*2);

	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &pid, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &cantPags, size_to_send);

	return serializedPackage;
}

int getLong_NuevoPrograma(){
	return sizeof(uint32_t)*2;
}


char* serializar_EscribirPagina(uint32_t pid, uint32_t numero_de_pagina, pagina pagina, int size_pagina){
	//mensaje: pid + numero_de_pagina + pagina
	char *serializedPackage = malloc(sizeof(uint32_t)*2+size_pagina);

	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &pid, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &numero_de_pagina, size_to_send);
	offset += size_to_send;

	size_to_send = size_pagina;
	memcpy(serializedPackage + offset, pagina, size_to_send);
	offset += size_to_send;

	return serializedPackage;
}

int getLong_EscribirPagina(int size_pagina){
	return sizeof(uint32_t)*2+size_pagina;
}


char* serializar_SolicitarPagina(uint32_t pid, uint32_t numero_de_pagina){
	//mensaje: pid + numero_de_pagina
	char *serializedPackage = malloc(sizeof(uint32_t)*2);

	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &pid, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &numero_de_pagina, size_to_send);

	return serializedPackage;
}

int getLong_SolicitarPagina(){
	return sizeof(uint32_t)*2;
}


char* serializar_EliminarPrograma(uint32_t pid){
	char *serializedPackage = malloc(sizeof(uint32_t));

	int size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage, &pid, size_to_send);

	return serializedPackage;
}

int getLong_EliminarPrograma(){
	return sizeof(uint32_t);
}


//TODO: borrar (solo para probar)
//hace un relleno de pagina con la letra pasada en "cad"
pagina llenarPagina(char* cad, int size){
	pagina page = malloc(sizeof(char)*size);
	int j;
	page[0] = '\0';
	for(j=0; j<size; j++){
		strcat(page,cad);
	}
	page[j-1] = '\0';
	return page;
}
