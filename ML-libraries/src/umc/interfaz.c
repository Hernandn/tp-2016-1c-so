/*
 * interfaz.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "interfaz.h"

int inicializar_programa(uint32_t pid, uint32_t paginas){
	char* parametros_serializados;
	Package* package;
	int resultado;

	logDebug("Se ha solicitado inicializar el programa %d con %d paginas.",pid,paginas);
	parametros_serializados=serializar_parametros(2,pid,paginas);
	enviarMensajeSocketConLongitud(*socket_umc,INICIALIZAR_PROGRAMA_UMC,parametros_serializados,sizeof(uint32_t)*2);

	recieve_and_deserialize(package,*socket_umc);
	if(package->msgCode==RESULTADO_OPERACION){
		resultado=*((int*)package->message);
	}

	free(parametros_serializados);
	return 0;
}

char* leer_pagina(uint32_t pagina, uint32_t offset, uint32_t tamanio){
	char* tmp;
	logDebug("Se solicito leer la de pagina %d con offset %d, %d bytes",pagina,offset,tamanio);
	return tmp;
}

int escribir_pagina(uint32_t pagina, uint32_t offset, uint32_t tamanio, char* buffer){
	logDebug("Se solicito escribir en la pagina %d offset $d, %d bytes. Contenido: %s",pagina,offset,tamanio,buffer);
	return 0;
}

int finalizar_programa(uint32_t pid){
	logDebug("Se solicito finalizar el programa %d",pid);
	return 0;
}

void definir_socket_umc(int* id){
	logDebug("Se define el socket %d para comunicacion con umc",id);
	if(*id){
		socket_umc = id;
	}
}

static char* serializar_parametros(int num,...){
	char* buffer=malloc(sizeof(uint32_t)*num);
	va_list valist;
	int i, offset=0;
	uint32_t tmp;

	va_start(valist, num);

	for (i=0; i<num; i++) {
		tmp=va_arg(valist, uint32_t);
		memcpy(buffer+offset,&tmp,sizeof(uint32_t));
		offset +=sizeof(uint32_t);
	}

	va_end(valist);
	return buffer;
}
