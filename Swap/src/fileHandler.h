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
void crearBitMap(int cantidadPaginas);
int getFirstAvailableBlock(int cantPaginas);
void escribirPaginaEnFrame(int frame, pagina pag, int sizePagina);
void escribirPaginasEnFrame(int frame, pagina* pag, int cantPaginas, int sizePagina);
int getFileSize(int sizePagina);
void cerrarArchivoSwap();
pagina leerPaginaFromFrame(int frame, int sizePagina);
void crearArchivoSwap(char* nombre);
void crearTablaDePaginas(int cantidadFrames);
void destroyTabla();
void ejemploPagina();
t_bitarray* getBitMap();
tableRow* getTablaDePaginas();
void guardarPrograma(int frame, int pid, int cantPaginas, pagina* paginas, int sizePagina);
void eliminarPrograma(int pid, int cantidadPaginas);
int buscarFramePorPagina(int pid, int pagina, int cantPaginas);
pagina leerPaginaDeProceso(int pid, int paginaNro, int cantPaginas, int sizePagina);
void escribirPaginaDeProceso(int pid, int paginaNro, pagina pag, int cantPaginas, int sizePagina);


#endif /* FILEHANDLER_H_ */
