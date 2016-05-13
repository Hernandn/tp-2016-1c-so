/*
 * client.h
 *
 *  Created on: 20/4/2016
 *      Author: utnso
 */

#include "package.h"
#ifndef CLIENT_CLIENT_H_
#define CLIENT_CLIENT_H_

int abrirConexionInetConServer(const char* ip, int port);
int leerSocketClient(int fd, char *datos, int longitud);
int escribirSocketClient(int fd, char *datos, int longitud);
char* serializarMensaje(Package *package);
int getLongitudPackage(Package *package);
Package* fillPackage(int msgCode, char* message);
void destroyPackage(Package* package);
void enviarMensajeSocket(int socket, int accion, char* mensaje);

#endif /* CLIENT_CLIENT_H_ */
