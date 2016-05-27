/*
 * interfaz.c
 *
 *  Created on: 26/5/2016
 *      Author: utnso
 */

#include "interfaz.h"

//funciones serializacion del PCB

char* serializarPCB(PCB* pcb){
	uint32_t size_metadata_program = getLong_metadata_program(pcb->codeIndex);
	uint32_t total_size = getLong_PCB(pcb);

	char *serializedPackage = malloc(total_size);

	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(pcb->processID), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(pcb->programCounter), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(pcb->pagesQty), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(size_metadata_program), size_to_send);
	offset += size_to_send;

	char* serialized_metadata = serializar_metadata_program(pcb->codeIndex);
	size_to_send = sizeof(char)*size_metadata_program;
	memcpy(serializedPackage + offset, serialized_metadata, size_to_send);
	offset += size_to_send;
	free(serialized_metadata);

	//TODO: borrar (solo para probar al principio le mando el programa hasta que funcione la UMC)
	size_to_send = strlen(pcb->programa)+1;
	memcpy(serializedPackage + offset, pcb->programa, size_to_send);

	return serializedPackage;
}

uint32_t getLong_PCB(PCB* pcb){
	uint32_t total_size = 0;
	uint32_t size_metadata_program = getLong_metadata_program(pcb->codeIndex);

	total_size += sizeof(uint32_t)*3;//PID + PC + pagesQty
	total_size += sizeof(uint32_t);//campo size_metadata_program
	total_size += size_metadata_program;

	//TODO: borrar (solo para probar al principio le mando el programa hasta que funcione la UMC)
	total_size += strlen(pcb->programa)+1;

	return total_size;
}

char* serializar_metadata_program(t_metadata_program* metadata){
	int total_size = getLong_metadata_program(metadata);

	char *serialized = malloc(total_size);

	int offset = 0;
	int size_to_send;

	//campo instruccion_inicio
	size_to_send = sizeof(metadata->instruccion_inicio);
	memcpy(serialized + offset, &(metadata->instruccion_inicio), size_to_send);
	offset += size_to_send;

	//campo instrucciones_size
	size_to_send = sizeof(metadata->instrucciones_size);
	memcpy(serialized + offset, &(metadata->instrucciones_size), size_to_send);
	offset += size_to_send;

	//array instrucciones_serializado
	int i;
	for(i=0; i < metadata->instrucciones_size; i++){
		size_to_send = sizeof(metadata->instrucciones_serializado[i].start);
		memcpy(serialized + offset, &(metadata->instrucciones_serializado[i].start), size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(metadata->instrucciones_serializado[i].offset);
		memcpy(serialized + offset, &(metadata->instrucciones_serializado[i].offset), size_to_send);
		offset += size_to_send;
	}

	//campo etiquetas_size
	size_to_send = sizeof(metadata->etiquetas_size);
	memcpy(serialized + offset, &(metadata->etiquetas_size), size_to_send);
	offset += size_to_send;

	//campo string etiquetas
	size_to_send = sizeof(char)*metadata->etiquetas_size;
	memcpy(serialized + offset, metadata->etiquetas, size_to_send);
	offset += size_to_send;

	//campo cantidad_de_funciones
	size_to_send = sizeof(int);
	memcpy(serialized + offset, &(metadata->cantidad_de_funciones), size_to_send);
	offset += size_to_send;

	//campo cantidad_de_etiquetas
	size_to_send = sizeof(int);
	memcpy(serialized + offset, &(metadata->cantidad_de_etiquetas), size_to_send);
	offset += size_to_send;

	return serialized;
}

uint32_t getLong_metadata_program(t_metadata_program* metadata){
	uint32_t total_size = 0;
	total_size += sizeof(metadata->instruccion_inicio);//inst_inicio
	total_size += sizeof(metadata->instrucciones_size);//size int32
	total_size += sizeof(t_intructions)*metadata->instrucciones_size;//size indice de codigo
	total_size += sizeof(metadata->etiquetas_size);//size etiquetas
	total_size += sizeof(char)*metadata->etiquetas_size;//string de etiquetas serializado
	total_size += sizeof(int)*2;//cant funciones y etiquetas
	return total_size;
}

PCB* deserializar_PCB(char* serialized){
	PCB* pcb = malloc(sizeof(PCB));
	int offset = 0;

	memcpy(&(pcb->processID),serialized+offset,sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(&(pcb->programCounter),serialized+offset,sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	memcpy(&(pcb->pagesQty),serialized+offset,sizeof(uint32_t));
	offset+=sizeof(uint32_t);

	int size_metadata_program;
	memcpy(&size_metadata_program,serialized+offset,sizeof(uint32_t));
	offset+=sizeof(uint32_t);

	char* serialized_metadata = malloc(sizeof(char)*size_metadata_program);
	memcpy(serialized_metadata,serialized+offset,size_metadata_program);
	offset+=size_metadata_program;

	pcb->codeIndex = deserializar_metadata_program(serialized_metadata);
	free(serialized_metadata);

	//TODO: borrar (solo para probar al principio le mando el programa hasta que funcione la UMC)
	pcb->programa = strdup(serialized+offset);

	return pcb;
}

t_metadata_program* deserializar_metadata_program(char* serialized){
	t_metadata_program* metadata = malloc(sizeof(t_metadata_program));
	int offset = 0;
	int size_aux;

	size_aux = sizeof(metadata->instruccion_inicio);
	memcpy(&(metadata->instruccion_inicio),serialized+offset,size_aux);
	offset+=size_aux;
	size_aux = sizeof(metadata->instrucciones_size);
	memcpy(&(metadata->instrucciones_size),serialized+offset,size_aux);
	offset+=size_aux;

	metadata->instrucciones_serializado = malloc(sizeof(t_intructions)*metadata->instrucciones_size);
	int i;
	for(i=0; i < metadata->instrucciones_size; i++){
		size_aux = sizeof(metadata->instrucciones_serializado[i].start);
		memcpy(&(metadata->instrucciones_serializado[i].start),serialized+offset,size_aux);
		offset+=size_aux;
		size_aux = sizeof(metadata->instrucciones_serializado[i].offset);
		memcpy(&(metadata->instrucciones_serializado[i].offset),serialized+offset,size_aux);
		offset+=size_aux;
	}

	size_aux = sizeof(metadata->etiquetas_size);
	memcpy(&(metadata->etiquetas_size),serialized+offset,size_aux);
	offset+=size_aux;

	metadata->etiquetas = malloc(sizeof(char)*metadata->etiquetas_size);
	memcpy(metadata->etiquetas,serialized+offset,sizeof(char)*metadata->etiquetas_size);

	size_aux = sizeof(metadata->cantidad_de_funciones);
	memcpy(&(metadata->cantidad_de_funciones),serialized+offset,size_aux);
	offset+=size_aux;
	size_aux = sizeof(metadata->cantidad_de_etiquetas);
	memcpy(&(metadata->cantidad_de_etiquetas),serialized+offset,size_aux);
	offset+=size_aux;

	return metadata;
}


void destroyPCB(PCB* self){
	free(self->programa);
	metadata_destruir(self->codeIndex);
	//free(self->stackIndex);
	//free(self->tagIndex);
	free(self);
}


//funciones interfaz CPU a Nucleo

void informarNucleoFinPrograma(int socketNucleo, PCB* pcb){
	char* serialized = serializarPCB(pcb);
	uint32_t longitud = getLong_PCB(pcb);
	enviarMensajeSocketConLongitud(socketNucleo,PROGRAM_FINISHED,serialized,longitud);
	free(serialized);
	//TODO: destruir PCB
}

void informarNucleoQuantumFinished(int socketNucleo, PCB* pcb){
	enviarMensajeSocket(socketNucleo,QUANTUM_FINISHED,string_itoa(pcb->processID));
}

void informarNucleoContextSwitchFinished(int socketNucleo, PCB* pcb){
	char* serialized = serializarPCB(pcb);
	uint32_t longitud = getLong_PCB(pcb);
	enviarMensajeSocketConLongitud(socketNucleo,CONTEXT_SWITCH_FINISHED,serialized,longitud);
	free(serialized);
	//TODO: destruir PCB
}

void informarNucleoCPUlibre(int socketNucleo){
	enviarMensajeSocket(socketNucleo,CPU_LIBRE,"");
}


//funciones interfaz Nucleo a CPU

void ejecutarNuevoProcesoCPU(int socketCPU, PCB* pcb){
	char* serialized = serializarPCB(pcb);
	int longitud = getLong_PCB(pcb);
	enviarMensajeSocketConLongitud(socketCPU,EXEC_NEW_PROCESS,serialized,longitud);
	free(serialized);
}

void continuarEjecucionProcesoCPU(int socketCPU){
	enviarMensajeSocket(socketCPU,CONTINUE_EXECUTION,"");
}
