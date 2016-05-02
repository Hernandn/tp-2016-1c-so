/*
 * log.h
 *
 *  Created on: 30/04/2016
 *      Author: hernan
 */

#include <stdbool.h>
#ifndef LOG_LOGGER_H_
#define LOG_LOGGER_H_

//Campos en archivo de configuracion
#define LOG_LEVEL "LOG_LEVEL"
#define LOG_FILE "LOG_FILE"
#define LOG_PROGRAM_NAME "LOG_PROGRAM_NAME"
#define LOG_PRINT_CONSOLE "LOG_PRINT_CONSOLE"

void logInfo(const char* string, ...);
void logDebug(const char* string, ...);
void logTrace(const char* string, ...);
void logWarning(const char* string, ...);
void logError(const char* string, ...);
void initLogMutex(char* file, char* program_name, bool is_active_console, t_log_level level);
void logDestroy();



#endif /* LOG_LOGGER_H_ */
