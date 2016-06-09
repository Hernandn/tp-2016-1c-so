/*
 * primitivas.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "primitivas.h"
#include "CPU.h"

static const int CONTENIDO_VARIABLE = 20;

contexto* getContextoActual(){
	return &(pcbActual->stackIndex[pcbActual->context_len-1]);
}

void crearVariable(t_nombre_variable variable_nom){
	contexto* contexto = getContextoActual();
	contexto->variables = realloc(contexto->variables,sizeof(variable)*(contexto->var_len+1));
	contexto->variables[contexto->var_len].nombre = variable_nom;
	contexto->variables[contexto->var_len].direccion.pagina = pcbActual->stackFirstPage;
	contexto->variables[contexto->var_len].direccion.offset = pcbActual->stackOffset;
	contexto->variables[contexto->var_len].direccion.size = sizeof(uint32_t);
	contexto->var_len++;
	pcbActual->stackOffset += sizeof(uint32_t);
}

void crearArgumento(t_nombre_variable variable_nom){
	contexto* contexto = getContextoActual();
	contexto->argumentos = realloc(contexto->argumentos,sizeof(variable)*(contexto->arg_len+1));
	contexto->argumentos[contexto->arg_len].nombre = variable_nom;
	contexto->argumentos[contexto->arg_len].direccion.pagina = pcbActual->stackFirstPage;
	contexto->argumentos[contexto->arg_len].direccion.offset = pcbActual->stackOffset;
	contexto->argumentos[contexto->arg_len].direccion.size = sizeof(uint32_t);
	contexto->arg_len++;
	pcbActual->stackOffset += sizeof(uint32_t);
}

dir_memoria* puntero_a_direccion_logica(t_puntero puntero){
	dir_memoria* dir = malloc(sizeof(dir_memoria));
	int pagina_num = puntero/size_pagina;
	int offset = puntero%size_pagina;
	dir->pagina = pcbActual->stackFirstPage + pagina_num;
	dir->offset = offset;
	dir->size = sizeof(uint32_t);
	return dir;
}

t_puntero direccion_logica_a_puntero(dir_memoria* dir){
	t_puntero puntero = 0;
	puntero += (dir->pagina - pcbActual->stackFirstPage);
	puntero += dir->offset;
	return puntero;
}



variable* obtener_variable(t_nombre_variable variable_nom){
	variable* var = NULL;
	contexto* contexto = getContextoActual();
	int i;
	for(i=0; i<contexto->var_len; i++){
		if(contexto->variables[i].nombre==variable_nom){
			var = &(contexto->variables[i]);
		}
	}
	return var;
}

variable* obtener_argumento(t_nombre_variable variable_nom){
	variable* var = NULL;
	contexto* contexto = getContextoActual();
	int i;
	for(i=0; i<contexto->arg_len; i++){
		if(contexto->argumentos[i].nombre==variable_nom){
			var = &(contexto->argumentos[i]);
		}
	}
	return var;
}


t_puntero_instruccion obtenerIndiceInstruccion(char* serialized, char* label, t_size size){
	int offset = 0;
	t_puntero_instruccion pos = -1;

	while(offset<size && strcmp(serialized+offset,label)!=0){
		offset = strlen(serialized+offset) + 1 + sizeof(t_puntero_instruccion);
	}

	if(offset<size){
		memcpy(&pos,serialized+strlen(serialized+offset)+1,sizeof(t_puntero_instruccion));

	}
	return pos;
}


//************************************************************
//						PRIMITIVAS
//************************************************************

t_puntero ml_definirVariable(t_nombre_variable variable_nom) {
	printf("Definir la variable %c\n", variable_nom);

	contexto* contexto = getContextoActual();
	if(isdigit(variable_nom)){
		crearArgumento(variable_nom);
		printf("Argumento definido: Nom: %c\n",contexto->argumentos[contexto->arg_len-1].nombre);
	} else {
		crearVariable(variable_nom);
		printf("Variable definida: Nom: %c\n",contexto->variables[contexto->var_len-1].nombre);
	}



	/*enviarMensajeSocket(getSocketUMC(),ALMACENAR_BYTES_PAGINA,"");
	printf("Enviando escritura de Bytes a UMC\n");
	analizarRespuestaUMC();*/

	t_puntero puntero = direccion_logica_a_puntero(&(contexto->variables[contexto->var_len-1].direccion));
	dir_memoria dir = contexto->variables[contexto->var_len-1].direccion;
	printf("Direccion logica a puntero: Pag:%d,Off:%d,Size:%d, puntero:%d\n",dir.pagina,dir.offset,dir.size,puntero);

	return puntero;
}

t_puntero ml_obtenerPosicionVariable(t_nombre_variable variable_nom) {
	printf("Obtener posicion de %c\n", variable_nom);
	/*printf("Enviando lectura de Bytes a UMC\n");
	enviarMensajeSocket(getSocketUMC(),SOLICITAR_BYTES_PAGINA,"");
	analizarRespuestaUMC();*/

	variable* variable = NULL;
	if(isdigit(variable_nom)){
		variable = obtener_argumento(variable_nom);
	} else {
		variable = obtener_variable(variable_nom);
	}

	dir_memoria* dir = &(variable->direccion);
	t_puntero puntero = direccion_logica_a_puntero(dir);
	printf("Direccion logica a puntero: Pag:%d,Off:%d,Size:%d, puntero:%d\n",dir->pagina,dir->offset,dir->size,puntero);

	return puntero;
}

t_valor_variable ml_dereferenciar(t_puntero puntero) {
	printf("Dereferenciar %d y su valor es: %d\n", puntero, CONTENIDO_VARIABLE);
	/*printf("Enviando lectura de Bytes a UMC\n");
	enviarMensajeSocket(getSocketUMC(),SOLICITAR_BYTES_PAGINA,"");
	analizarRespuestaUMC();*/

	dir_memoria* dir = puntero_a_direccion_logica(puntero);
	printf("Puntero a Direccion logica: puntero:%d, Pag:%d,Off:%d,Size:%d\n",puntero,dir->pagina,dir->offset,dir->size);
	free(dir);

	return CONTENIDO_VARIABLE;
}

void ml_asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignando en %d el valor %d\n", puntero, variable);
	/*printf("Enviando escritura de Bytes a UMC\n");
	enviarMensajeSocket(getSocketUMC(),ALMACENAR_BYTES_PAGINA,"");
	analizarRespuestaUMC();*/

	dir_memoria* dir = puntero_a_direccion_logica(puntero);
	printf("Puntero a Direccion logica: puntero:%d, Pag:%d,Off:%d,Size:%d\n",puntero,dir->pagina,dir->offset,dir->size);
	free(dir);
}

void ml_imprimir(t_valor_variable valor) {
	printf("Imprimir %d\n", valor);
	informarNucleoImprimirVariable(socketNucleo,pcbActual->processID,valor);
}

void ml_imprimirTexto(char* texto) {
	printf("ImprimirTexto: %s", texto);
	informarNucleoImprimirTexto(socketNucleo,pcbActual->processID,texto);
}

t_valor_variable ml_obtenerValorCompartida(t_nombre_compartida variable){
	printf("\nObteniendo Valor Variable compartida: %s\n",variable);
	return CONTENIDO_VARIABLE;
}

t_valor_variable ml_asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	printf("\nAsignando Valor Variable compartida: %s, valor: %d\n",variable,valor);
	return valor;
}

void ml_irAlLabel(t_nombre_etiqueta t_nombre_etiqueta){
	printf("\nEjecutando Ir a Label: %s\n",t_nombre_etiqueta);
}

void ml_llamarSinRetorno(t_nombre_etiqueta etiqueta){
	printf("\nEjecutando Llamar sin retorno a funcion: %s\n",etiqueta);
}

void ml_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	printf("\nEjecutando Llamar con retorno a funcion: %s, Retorno: %d\n",etiqueta,donde_retornar);
}

void ml_finalizar(void){
	printf("\nEjecutando Finalizar\n");
}

void ml_retornar(t_valor_variable retorno){
	printf("\nEjecutando Retornar, valor de retorno: %d\n",retorno);
}

void ml_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){
	printf("\nEjecutando Entrada-Salida [Dispositivo: %s, Tiempo: %d]\n",dispositivo,tiempo);
	ejecutarOperacionIO(dispositivo,tiempo);
}

void ml_wait(t_nombre_semaforo identificador_semaforo){
	printf("\nEjecutando Wait de semaforo: %s\n",identificador_semaforo);
}

void ml_signal(t_nombre_semaforo identificador_semaforo){
	printf("\nEjecutando Signal de semaforo: %s\n",identificador_semaforo);
}
