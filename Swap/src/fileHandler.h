/*
 * fileHandler.h
 *
 *  Created on: 5/5/2016
 *      Author: utnso
 */

#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include "configuration.h"

typedef char* pagina;

//representa una fila en la tabla de paginas del Swap (estructura de control)
typedef struct tableRow {
	int pid;
	int page;
}tableRow;

void inicializarSwap(Configuration* config);
t_bitarray* crearBitMap(int cantidadPaginas);
int getFirstAvailableBlock(int cantPaginas);
void escribirPaginaEnFrame(int frame, pagina pag, int sizePagina);
void escribirPaginasEnFrame(int frame, pagina* pag, int cantPaginas, int sizePagina);
int getFileSize(int sizePagina);
void cerrarArchivoSwap();
pagina leerPaginaFromFrame(int frame, int sizePagina);
FILE* crearArchivoSwap(char* nombre);
tableRow* crearTablaDePaginas(int cantidadFrames);
void destroyTabla();
void ejemploPagina();
t_bitarray* getBitMap();


#endif /* FILEHANDLER_H_ */
