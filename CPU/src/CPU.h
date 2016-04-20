/*
 * CPU.h
 *
 *  Created on: 19/4/2016
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

//codigos de operaciones de la UMC
//esto tenemos que ver bien despues que decidimos hacer para codificarlas
#define INIT_PROGRAM 1
#define SOLICITAR_BYTES_PAGINA 2
#define ALMACENAR_BYTES_PAGINA 3
#define END_PROGRAM 4
#define HANDSHAKE 5
#define SWITCH_PROCESS 6
//---------------------


#define MAX_MESSAGE_SIZE 300

/*
 * 	Es importante destacar que utilizamos el tipo uint_32, incluida en el header <stdint.h> para mantener un estandar en la cantidad
 * 	de bytes del paquete.
 */
typedef struct Package {
	uint32_t msgCode;
	uint32_t message_long;
	char message[MAX_MESSAGE_SIZE];
} Package;

#endif /* CPU_H_ */