/*
 * log.h
 *
 *  Created on: 30/04/2016
 *      Author: hernan
 */

#ifndef LOG_H_
#define LOG_H_

void logInfo(const char* string);
void logDebug(const char* string);
void logTrace(const char* string);
void logWarning(const char* string);
void logError(const char* string);
void initLogMutex();

#endif /* LOG_H_ */
