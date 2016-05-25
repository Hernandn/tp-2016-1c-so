/*
 * interfaz.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "interfaz.h"

static char* serializar_parametros(int num,...){

	char* buffer=malloc(sizeof(uint32_t)*num);
	va_list valist;
	int i, offset=0;
	uint32_t tmp;

	va_start(valist, num);

	for(i=0; i<num; i++){
		tmp=va_arg(valist, uint32_t);
		memcpy(buffer+offset,&tmp,sizeof(uint32_t));
		offset +=sizeof(uint32_t);
	}

	va_end(valist);
	return buffer;
}

void definir_socket_umc(int* id){

	/* El objetivo de esta funcion es versatilidad.
	 * Los proyectos que usen esta biblioteca no tienen
	 * que estar pendientes de como se llama la variable
	 * que contiene el id del socket con la umc.
	 */
	logDebug("Se define el socket %d para comunicacion con umc",id);
	socket_umc = id;
}

int inicializar_programa(uint32_t pid, uint32_t paginas){

	char* parametros_serializados;
	Package* package;
	uint32_t resultado=-1;

	logDebug("Se ha solicitado inicializar el programa %d con %d paginas.",pid,paginas);
	parametros_serializados=serializar_parametros(2,pid,paginas);

	enviarMensajeSocketConLongitud(*socket_umc,INICIALIZAR_PROGRAMA_UMC,parametros_serializados,sizeof(uint32_t)*2);
	if(recieve_and_deserialize(package,*socket_umc) > 0)
	{
		if(package->msgCode==RESULTADO_OPERACION){
			memcpy(&resultado,package->message,sizeof(uint32_t));
		}
	}

	free(parametros_serializados);
	destroyPackage(package);

	return resultado;
}

char* leer_pagina(uint32_t pagina, uint32_t tamanio){

	char *buffer=NULL,
		 *parametros_serializados;
	Package* package;

	logDebug("Se solicito leer %d bytes de la pagina %d",tamanio,pagina);
	parametros_serializados=serializar_parametros(2,pagina,tamanio);

	enviarMensajeSocketConLongitud(*socket_umc,LEER_PAGINA_UMC,parametros_serializados,sizeof(uint32_t)*2);
	if(recieve_and_deserialize(package,*socket_umc) > 0)
	{
		if(package->msgCode==RESULTADO_OPERACION){
			buffer=package->message;
		}
	}

	free(parametros_serializados);
	/* TODO
	 * Aca tendria que liberar la memoria del paquete
	 * pero como todavia se bien que devuelve la umc
	 * no puedo darle un tratamiento mas copado
	 */
	//destroyPackage(package);

	return buffer;
}

int escribir_pagina(uint32_t pagina, uint32_t tamanio, char* buffer){

	/*TODO No esta lista, todavia no se buen como voy a serializar el buffer*/

	char* parametros_serializados;
	Package* package;
	uint32_t resultado=-1;

	logDebug("Se solicito escribir %d bytes en la pagina %d. Contenido: %s",tamanio,pagina,buffer);
	parametros_serializados=serializar_parametros(2,pagina,tamanio);

	enviarMensajeSocketConLongitud(*socket_umc,ESCRIBIR_PAGINA_UMC,parametros_serializados,sizeof(uint32_t)*2);
	if(recieve_and_deserialize(package,*socket_umc) > 0)
	{
		if(package->msgCode==RESULTADO_OPERACION){
			memcpy(&resultado,package->message,sizeof(uint32_t));
		}
	}

	free(parametros_serializados);
	destroyPackage(package);

	return resultado;
}

int finalizar_programa(uint32_t pid){

	char* parametros_serializados;
	Package* package;
	uint32_t resultado=-1;

	logDebug("Se solicito finalizar el programa %d",pid);
	parametros_serializados=serializar_parametros(1,pid);

	enviarMensajeSocketConLongitud(*socket_umc,FINALIZAR_PROGRAMA_UMC,parametros_serializados,sizeof(uint32_t)*1);
	if(recieve_and_deserialize(package,*socket_umc) > 0)
	{
		if(package->msgCode==RESULTADO_OPERACION){
			memcpy(&resultado,package->message,sizeof(uint32_t));
		}
	}

	free(parametros_serializados);
	destroyPackage(package);

	return resultado;
}
