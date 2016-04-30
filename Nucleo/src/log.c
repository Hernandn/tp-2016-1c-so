/*
 * log.c
 *
 *  Created on: 30/04/2016
 *      Author: hernan
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/log.h>

pthread_mutex_t logMutex;
t_log* logger2;

void initLogMutex(){
	pthread_mutex_init(&logMutex,NULL);
	logger2 = log_create("nucleo2.log","ELESTAC",true, LOG_LEVEL_DEBUG);
}

void logInfo(const char* string){
	pthread_mutex_lock(&logMutex);
	log_info(logger2,string);
	pthread_mutex_unlock(&logMutex);
}

void logDebug(const char* string){
	pthread_mutex_lock(&logMutex);
	log_debug(logger2,string);
	pthread_mutex_unlock(&logMutex);
}

void logTrace(const char* string){
	pthread_mutex_lock(&logMutex);
	log_trace(logger2,string);
	pthread_mutex_unlock(&logMutex);
}

void logWarning(const char* string){
	pthread_mutex_lock(&logMutex);
	log_warning(logger2,string);
	pthread_mutex_unlock(&logMutex);
}

void logError(const char* string){
	pthread_mutex_lock(&logMutex);
	log_error(logger2,string);
	pthread_mutex_unlock(&logMutex);
}
