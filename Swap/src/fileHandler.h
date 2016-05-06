/*
 * fileHandler.h
 *
 *  Created on: 5/5/2016
 *      Author: utnso
 */

#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/bitarray.h>

typedef char* pagina;

t_bitarray* crearBitMap(int cantidadPaginas);
void escribirPaginaEnFrame(FILE* file, int frame, pagina* pag, int sizePagina);
int getFileSize(FILE* file, int sizePagina);
void cerrarArchivoSwap(FILE* file);
pagina leerPaginaFromFrame(FILE* file, int frame, int sizePagina);
FILE* crearArchivoSwap(char* nombre);
void ejemploPagina();

#endif /* FILEHANDLER_H_ */
