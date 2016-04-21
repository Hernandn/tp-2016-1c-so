/*
 * client.h
 *
 *  Created on: 20/4/2016
 *      Author: utnso
 */

#include "package.h"
#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

int abrirConexionInet(const char* ip, int port);
int leerSocketClient(int fd, char *datos, int longitud);
int escribirSocketClient(int fd, char *datos, int longitud);
char* serializarMensaje(Package *package);
void fillPackage(Package *package, int msgCode, char* message);
int getLongitudPackage(Package *package);

#endif /* CLIENT_CLIENT_H_ */
