/*
 * comandos.c
 *
 *  Created on: 5/6/2016
 *      Author: utnso
 */

#include "comandos.h"

void handleComandos(){

	char * comando;
	size_t size_buff=0;

	//Mutex para manejar el flag de ejecucion
	pthread_mutex_init(&continua_mutex,NULL);

	pthread_mutex_lock(&continua_mutex);
	while(continua){

		comando = NULL;
		printf("ml-umc> ");
		getline(&comando,&size_buff,stdin);
		intepretarComando(comando);

		free(comando);
	}
	pthread_mutex_unlock(&continua_mutex);
}

void intepretarComando(char* comando){

	char** comando_parseado = malloc(sizeof(char*)*2);
	int cantidad;

	//TODO no estoy liberando comando_parseado en ningun momento
	cantidad=parsear_comando(comando, comando_parseado);

	if(!strcmp(*comando_parseado,"dump")) dump();
	else if(!strcmp(*comando_parseado,"flush") && (cantidad == 2))

			if(!strcmp(*(comando_parseado+1),"tlb")) flush_tlb();
			else if (!strcmp(*(comando_parseado+1),"memory")) flush_memory();
				 else error_comando(comando);

		 else if(!strcmp(*comando_parseado,"retardo"))

			if(cantidad == 2) retardo(atoi(*(comando_parseado + 1)));
			else print_retardo();

			else if(!strcmp(*comando_parseado,"clear")) limpiar_pantalla();
				 else if(!strcmp(*comando_parseado,"exit")) fin_programa();
				 	  else error_comando(comando);
}

int parsear_comando(char * comando, char ** comando_parseado){

	int i=0;				//Indice para recorrer el string
	int contador = 0;		//Contador de palabras parseadas
	char * tmp = comando;	//Puntero al comienzo de la proxima palabra a parsear
	int letras = 0;			//Conador de letras de la palara a parsear

	while(*(comando + i) != '\0'){
		if (*(comando + i) == 32){
			if(letras != 0){
				*(comando_parseado + contador) = malloc(sizeof(char)*letras+1);
				strncpy(*(comando_parseado + contador),tmp,letras);
				*(*(comando_parseado + contador)+letras) = '\0';
				contador ++;
			}
			tmp=tmp+i+1;
			letras = 0;
		}else {
			letras++;
		}

		i++;
	}

	*(comando_parseado + contador)= malloc(sizeof(char)*letras+1);
	strncpy(*(comando_parseado + contador),tmp,letras);

	//getline tambien guarda el \n y hay que eliminarlo para poder comparar despues
	*(*(comando_parseado + contador)+letras-1) = '\0';

	return contador+1;
}

void dump ()
{
	printf("Este es un reporte de prueba del estado dump\n");

}

void retardo (int segundos)
{
	pthread_mutex_lock(&retardo_mutex);
	config->retraso = segundos;
	pthread_mutex_unlock(&retardo_mutex);
}

void print_retardo(){
	pthread_mutex_lock(&retardo_mutex);
	printf("Retardo actual: %d\n",config->retraso);
	pthread_mutex_unlock(&retardo_mutex);
}

void flush_tlb()
{

	printf("hola soy el flush tlb\n");
}

void flush_memory()
{
	printf("hola soy el flush memory\n");
}

void error_comando(char* comando)
{
	printf("Comando inexistente %s", comando);
}

void limpiar_pantalla(){

	system("clear");

}

void fin_programa(){

	/*Se cambia la variable "continua" a 0 para que
	 * todos los threads sepa que se ejecuto el comando exit
	 */

	continua = 0;

}
