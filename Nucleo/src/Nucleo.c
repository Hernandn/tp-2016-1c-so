/*
 ============================================================================
 Name        : Nucleo.c
 Author      : Andres Michel
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/errno.h>


int sockfd;
struct sockaddr_in server;
//static portnum = 0;

int main(void) {

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = 0;

	// si no puede crear el socket pasa esto
	if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("fallo abriendo el socket \n");
		exit(1);
	}

	// hago el blind del srv
	if (bind(sockfd, (struct sockaddr*)&server, sizeof(server)))
	{
		perror("no se hizo el blind");
		exit(1);
	}

	// activo modo escucha
	if(listen(sockfd, 5) < 0)
	{
		//printf("no se pudo escuchar en el puerto %d : %s \n", portnum, sys_errlist[errno]);
		exit(1);
	}

	close(sockfd);
}


