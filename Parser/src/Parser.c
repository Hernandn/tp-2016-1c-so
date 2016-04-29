
/*
 ============================================================================
 Name        : Nicolas Abbiuso
 Author      : 
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
//#include "CPU.h"
#include <parser/metadata_program.h>

/*Como primera instancia muestro por pantalla los pasos del parser para luego
aplicar la comunicacion con la UMC. Ej. a=b+3. Los pasos son los siguientes:
1) Obtener direccion de 'b'
2) Obtener valor de 'b' (Se lo pide a UMC)
3) Obtener direccion de 'a'
4) Almacenar (Lo hara la UMC)   */


int main (void) {
printf("Como primera instancia muestro por pantalla los pasos del parser para luego aplicar la comunicacion con la UMC. Ej. a=b+3 \n");
printf("Obtengo direccion de 'b'\n");
printf ("Obtengo valor de 'b' pidiendoselo a la UMC\n");
printf ("Obtengo direccion de 'a'\n");
printf ("Almaceno (UMC)");

}
