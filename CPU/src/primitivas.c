/*
 * primitivas.c
 *
 *  Created on: 19/5/2016
 *      Author: utnso
 */

#include "primitivas.h"

static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;

t_puntero ml_definirVariable(t_nombre_variable variable) {
	printf("Definir la variable %c\n", variable);
	return POSICION_MEMORIA;
}

t_puntero ml_obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtener posicion de %c\n", variable);
	return POSICION_MEMORIA;
}

t_valor_variable ml_dereferenciar(t_puntero puntero) {
	printf("Dereferenciar %d y su valor es: %d\n", puntero, CONTENIDO_VARIABLE);
	return CONTENIDO_VARIABLE;
}

void ml_asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignando en %d el valor %d\n", puntero, variable);
}

void ml_imprimir(t_valor_variable valor) {
	printf("Imprimir %d\n", valor);
}

void ml_imprimirTexto(char* texto) {
	printf("ImprimirTexto: %s", texto);
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
}

void ml_wait(t_nombre_semaforo identificador_semaforo){
	printf("\nEjecutando Wait de semaforo: %s\n",identificador_semaforo);
}

void ml_signal(t_nombre_semaforo identificador_semaforo){
	printf("\nEjecutando Signal de semaforo: %s\n",identificador_semaforo);
}
