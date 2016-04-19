/*
 ============================================================================
 Name        : CPU.c
 Author      : Andres Michel
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

int abrirConexionInet();
int leerSocket(int fd, char *datos, int longitud);
int escribirSocket(int fd, char *datos, int longitud);

int main(void){

	int socket;		/* descriptor de conexión con el servidor */
	int buffer;		/* buffer de lectura de datos procedentes del servidor */
	int error;		/* error de lectura por el socket */

	/* Se abre una conexión con el servidor */
	socket = abrirConexionInet();

	/* Se lee el número de cliente, dato que nos da el servidor. Se escribe
	 * dicho número en pantalla.*/
	error = leerSocket(socket, (char *)&buffer, sizeof(int));

	/* Si ha habido error de lectura lo indicamos y salimos */
	if (error < 1)
	{
		printf ("Me han cerrado la conexión\n");
		exit(-1);
	}

	/* Se escribe el número de cliente que nos ha enviado el servidor */
	printf ("Soy cliente %d\n", buffer);

	/* Bucle infinito. Envia al servidor el número de cliente y espera un
	 * segundo */
	while (1)
	{
		escribirSocket(socket, (char *)&buffer, sizeof(int));
		sleep(3);
	}
}

/*
* Conecta con un servidor remoto a traves de socket INET
*/
int abrirConexionInet()
{
	int socketFD;
	struct sockaddr_in serv_addr;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(8080);

	socketFD = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socketFD == -1)
		return -1;

	if (connect(socketFD, (void*) &serv_addr, sizeof(serv_addr)) == -1) {
		perror("La CPU no se pudo conectar");
		return -1;
	}

	return socketFD;
}

int leerSocket(int fd, char *datos, int longitud)
{
	int leido = 0;
	int aux = 0;

	/*
	* Comprobacion de que los parametros de entrada son correctos
	*/
	if ((fd == -1) || (datos == NULL) || (longitud < 1))
		return -1;

	/*
	* Mientras no hayamos leido todos los datos solicitados
	*/
	while (leido < longitud)
	{
		aux = read (fd, datos + leido, longitud - leido);
		if (aux > 0)
		{
			/*
			* Si hemos conseguido leer datos, incrementamos la variable
			* que contiene los datos leidos hasta el momento
			*/
			leido = leido + aux;
		}
		else
		{
			/*
			* Si read devuelve 0, es que se ha cerrado el socket. Devolvemos
			* los caracteres leidos hasta ese momento
			*/
			if (aux == 0)
				return leido;
			if (aux == -1)
			{
				/*
				* En caso de error, la variable errno nos indica el tipo
				* de error.
				* El error EINTR se produce si ha habido alguna
				* interrupcion del sistema antes de leer ningun dato. No
				* es un error realmente.
				* El error EGAIN significa que el socket no esta disponible
				* de momento, que lo intentemos dentro de un rato.
				* Ambos errores se tratan con una espera de 100 microsegundos
				* y se vuelve a intentar.
				* El resto de los posibles errores provocan que salgamos de
				* la funcion con error.
				*/
				switch (errno)
				{
					case EINTR:
					case EAGAIN:
						usleep (100);
						break;
					default:
						return -1;
				}
			}
		}
	}

	/*
	* Se devuelve el total de los caracteres leidos
	*/
	return leido;
}


/*
* Escribe dato en el socket cliente. Devuelve numero de bytes escritos,
* o -1 si hay error.
*/
int escribirSocket(int fd, char *datos, int longitud)
{
	int escrito = 0;
	int aux = 0;

	/*
	* Comprobacion de los parametros de entrada
	*/
	if ((fd == -1) || (datos == NULL) || (longitud < 1))
		return -1;

	/*
	* Bucle hasta que hayamos escrito todos los caracteres que nos han
	* indicado.
	*/
	while (escrito < longitud)
	{
		aux = write (fd, datos + escrito, longitud - escrito);
		if (aux > 0)
		{
			/*
			* Si hemos conseguido escribir caracteres, se actualiza la
			* variable escrito
			*/
			escrito = escrito + aux;
		}
		else
		{
			/*
			* Si se ha cerrado el socket, devolvemos el numero de caracteres
			* leidos.
			* Si ha habido error, devolvemos -1
			*/
			if (aux == 0)
				return escrito;
			else
				return -1;
		}
	}

	/*
	* Devolvemos el total de caracteres leidos
	*/
	return escrito;
}
