/*
 * interfaz.c
 *
 *  Created on: 26/5/2016
 *      Author: utnso
 */

#include "interfaz.h"

//funciones serializacion del PCB

char* serializarPCB(PCB* pcb){
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
	memcpy(serializedPackage + offset, &(pcb->stackFirstPage), size_to_send);
	offset += size_to_send;

	//serializar code index
	uint32_t size_metadata_program = getLong_metadata_program(pcb->codeIndex);
	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(size_metadata_program), size_to_send);
	offset += size_to_send;

	char* serialized_metadata = serializar_metadata_program(pcb->codeIndex);
	size_to_send = sizeof(char)*size_metadata_program;
	memcpy(serializedPackage + offset, serialized_metadata, size_to_send);
	offset += size_to_send;
	free(serialized_metadata);

	//serializar stack
	uint32_t size_stack = getLong_stack(pcb->stackIndex);
	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(size_stack), size_to_send);
	offset += size_to_send;

	char* serialized_stack = serializar_stack(pcb->stackIndex);
	size_to_send = sizeof(char)*size_stack;
	memcpy(serializedPackage + offset, serialized_stack, size_to_send);
	offset += size_to_send;
	free(serialized_stack);

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
	memcpy(&(pcb->stackFirstPage),serialized+offset,sizeof(uint32_t));
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
	stack_destroy_and_destroy_elements(self->stackIndex,(void*)destroy_contexto);
	//free(self->stackIndex);
	//free(self->tagIndex);
	free(self);
}

//envio la solicitud IO mas el PCB entero para hacer el context switch y que se bloquee en el Nucleo
char* serializar_ejecutarOperacionIO(PCB* pcb, char* io_id, uint32_t cant_operaciones){
	uint32_t size_pcb = getLong_PCB(pcb);
	uint32_t total_size = getLong_ejecutarOperacionIO(pcb,io_id,cant_operaciones);

	char *serializedPackage = malloc(total_size);

	int offset = 0;
	int size_to_send;

	uint32_t io_id_length = strlen(io_id)+1;
	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(io_id_length), size_to_send);
	offset += size_to_send;

	size_to_send = strlen(io_id)+1;
	memcpy(serializedPackage + offset, io_id, size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(cant_operaciones), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(size_pcb), size_to_send);
	offset += size_to_send;

	char* serialized_pcb = serializarPCB(pcb);
	size_to_send = sizeof(char)*size_pcb;
	memcpy(serializedPackage + offset, serialized_pcb, size_to_send);
	offset += size_to_send;
	free(serialized_pcb);

	return serializedPackage;
}

uint32_t getLong_ejecutarOperacionIO(PCB* pcb, char* io_id, uint32_t cant_operaciones){
	return strlen(io_id)+1 + sizeof(uint32_t)*3 + getLong_PCB(pcb);
}

solicitud_io* deserializar_ejecutarOperacionIO(char* serialized){
	solicitud_io* sol = malloc(sizeof(solicitud_io));
	int offset = 0;

	uint32_t io_id_length;
	memcpy(&(io_id_length),serialized+offset,sizeof(uint32_t));
	offset+=sizeof(uint32_t);
	sol->io_id = strdup(serialized+offset);
	offset+=io_id_length;
	memcpy(&(sol->cant_operaciones),serialized+offset,sizeof(uint32_t));
	offset+=sizeof(uint32_t);

	int size_pcb;
	memcpy(&size_pcb,serialized+offset,sizeof(uint32_t));
	offset+=sizeof(uint32_t);

	char* serialized_pcb = malloc(sizeof(char)*size_pcb);
	memcpy(serialized_pcb,serialized+offset,size_pcb);
	offset+=size_pcb;

	sol->pcb = deserializar_PCB(serialized_pcb);
	free(serialized_pcb);

	return sol;
}


char* serializar_contexto(contexto* contexto){
	uint32_t total_size = getLong_contexto(contexto);
	char *serializedPackage = malloc(total_size);

	int offset = 0;
	int size_to_send;

	//cantidad de argumentos
	uint32_t argumentos_length = dictionary_size(contexto->argumentos);
	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(argumentos_length), size_to_send);
	offset += size_to_send;

	//serializar diccionario de argumentos
	char* serialized_dictionary = serializar_dictionary(contexto->argumentos);
	uint32_t size_dictionary = getLong_dictionary(contexto->argumentos);
	size_to_send = sizeof(char)*size_dictionary;
	memcpy(serializedPackage + offset, serialized_dictionary, size_to_send);
	offset += size_to_send;
	free(serialized_dictionary);

	//cantidad de variables
	uint32_t variables_length = dictionary_size(contexto->variables);
	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(variables_length), size_to_send);
	offset += size_to_send;

	//serializar diccionario de variables
	serialized_dictionary = serializar_dictionary(contexto->variables);
	size_dictionary = getLong_dictionary(contexto->variables);
	size_to_send = sizeof(char)*size_dictionary;
	memcpy(serializedPackage + offset, serialized_dictionary, size_to_send);
	offset += size_to_send;
	free(serialized_dictionary);

	//posicion de retorno
	size_to_send = sizeof(t_puntero_instruccion);
	memcpy(serializedPackage + offset, &(contexto->retPos), size_to_send);
	offset += size_to_send;

	//direccion de variable de retorno
	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(contexto->retVar.pagina), size_to_send);
	offset += size_to_send;
	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(contexto->retVar.offset), size_to_send);
	offset += size_to_send;
	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(contexto->retVar.size), size_to_send);
	offset += size_to_send;

	return serializedPackage;
}

uint32_t getLong_contexto(contexto* contexto){
	uint32_t longitud = 0;
	longitud += getLong_dictionary(contexto->argumentos);
	longitud += getLong_dictionary(contexto->variables);
	longitud += sizeof(contexto->retPos);
	longitud += sizeof(contexto->retVar);
	return longitud;
}

char* serializar_dictionary(t_dictionary* dictionary){
	uint32_t total_size = getLong_dictionary(dictionary);
	char *serializedPackage = malloc(total_size);

	int offset = 0;
	int size_to_send;
	int i;
	for (i = 0; i < dictionary->table_max_size; i++) {
		t_hash_element *element = dictionary->elements[i];

		while (element != NULL) {
			char key = *(element->key);
			size_to_send = sizeof(char);
			memcpy(serializedPackage + offset, &(key), size_to_send);
			offset += size_to_send;
			dir_memoria* dir = element->data;
			size_to_send = sizeof(uint32_t);
			memcpy(serializedPackage + offset, &(dir->pagina), size_to_send);
			offset += size_to_send;
			size_to_send = sizeof(uint32_t);
			memcpy(serializedPackage + offset, &(dir->offset), size_to_send);
			offset += size_to_send;
			size_to_send = sizeof(uint32_t);
			memcpy(serializedPackage + offset, &(dir->size), size_to_send);
			offset += size_to_send;

			element = element->next;
		}
	}
	return serializedPackage;
}

uint32_t getLong_dictionary(t_dictionary* dictionary){
	return (sizeof(char)+sizeof(dir_memoria))*dictionary_size(dictionary);
}

char* serializar_stack(t_stack* stack){
	uint32_t total_size = getLong_stack(stack);
	char *serializedPackage = malloc(total_size);

	int offset = 0;
	int size_to_send;

	//cantidad de contextos
	uint32_t contextos_length = stack_size(stack);
	size_to_send = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(contextos_length), size_to_send);
	offset += size_to_send;

	int i;
	for (i = 0; i < stack_size(stack); i++) {
		contexto* contexto = list_get(stack->elements,i);
		char* serialized_contexto = serializar_contexto(contexto);
		uint32_t size_contexto = getLong_contexto(contexto);
		size_to_send = sizeof(char)*size_contexto;
		memcpy(serializedPackage + offset, serialized_contexto, size_to_send);
		offset += size_to_send;
		free(serialized_contexto);
	}
	return serializedPackage;
}

uint32_t getLong_stack(t_stack* stack){
	uint32_t total = 0;
	int i;
	for(i=0; i<stack_size(stack); i++){
		t_dictionary* dictionary = list_get(stack->elements,i);
		total += getLong_dictionary(dictionary);
	}
	return total;
}


void crearNuevoContexto(PCB* pcb){
	contexto* contexto = malloc(sizeof(contexto));
	contexto->argumentos = dictionary_create();
	contexto->variables = dictionary_create();

	stack_push(pcb->stackIndex,contexto);
}

void destroy_contexto(contexto* contexto){
	dictionary_destroy_and_destroy_elements(contexto->argumentos,(void*)destroy_dir_memoria);
	dictionary_destroy_and_destroy_elements(contexto->variables,(void*)destroy_dir_memoria);
	free(contexto);
}

void destroy_dir_memoria(dir_memoria* dir){
	free(dir);
}


//funciones interfaz CPU a Nucleo

void informarNucleoFinPrograma(int socketNucleo, PCB* pcb){
	char* serialized = serializarPCB(pcb);
	uint32_t longitud = getLong_PCB(pcb);
	enviarMensajeSocketConLongitud(socketNucleo,PROGRAM_FINISHED,serialized,longitud);
	free(serialized);
}

void informarNucleoQuantumFinished(int socketNucleo, PCB* pcb){
	enviarMensajeSocket(socketNucleo,QUANTUM_FINISHED,string_itoa(pcb->processID));
}

void informarNucleoContextSwitchFinished(int socketNucleo, PCB* pcb){
	char* serialized = serializarPCB(pcb);
	uint32_t longitud = getLong_PCB(pcb);
	enviarMensajeSocketConLongitud(socketNucleo,CONTEXT_SWITCH_FINISHED,serialized,longitud);
	free(serialized);
}

void informarNucleoCPUlibre(int socketNucleo){
	enviarMensajeSocket(socketNucleo,CPU_LIBRE,"");
}

void informarNucleoEjecutarOperacionIO(int socketNucleo, PCB* pcb, char* io_id, uint32_t cant_operaciones){
	char* serialized = serializar_ejecutarOperacionIO(pcb,io_id,cant_operaciones);
	uint32_t longitud = getLong_ejecutarOperacionIO(pcb,io_id,cant_operaciones);
	enviarMensajeSocketConLongitud(socketNucleo,EXEC_IO_OPERATION,serialized,longitud);
	free(serialized);
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
