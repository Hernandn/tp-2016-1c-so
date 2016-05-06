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

t_bitarray* crearBitMap(int cantidadPaginas);
int getFirstAvailableBlock(FILE* file, t_bitarray* bitMap, int cantPaginas);
void escribirPaginaEnFrame(FILE* file, t_bitarray* bitMap, int frame, pagina pag, int sizePagina);
void escribirPaginasEnFrame(FILE* file, t_bitarray* bitMap, int frame, pagina* pag, int cantPaginas, int sizePagina);
int getFileSize(FILE* file, int sizePagina);
void cerrarArchivoSwap(FILE* file);
pagina leerPaginaFromFrame(FILE* file, int frame, int sizePagina);
void setPaginasOcupadas(t_bitarray* bitMap, int offset, int cantidad);
FILE* crearArchivoSwap(char* nombre);
tableRow* crearTablaDePaginas(int cantidadFrames);
void destroyTabla(tableRow* table);
void ejemploPagina();


#endif /* FILEHANDLER_H_ */
