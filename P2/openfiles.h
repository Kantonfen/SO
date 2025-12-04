#ifndef OPENFILES_H
#define OPENFILES_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  // open, O_*
#include <string.h>
#include <unistd.h> // close, dup

typedef struct {
    int fd;          // descriptor del archivo , entero unico que asigna el sistema despues al hacer open
    char *nombre;    
    char *modo;      
} ArchivoAbierto;

typedef struct {
    ArchivoAbierto *data; //array dinamico de archivos
    int tamano;             //cuantos hay abiertos
    int capacidad;          //tamaño maximo
} OpenFiles;


void initOpenFiles(OpenFiles *l);
void freeOpenFiles(OpenFiles *l);
void addOpenFile(OpenFiles *l, int fd, const char *nombre, const char *modo);
void removeOpenFile(OpenFiles *l, int fd);
void printOpenFiles(const OpenFiles *l);


ArchivoAbierto *findOpenFile(OpenFiles *l, int fd);

#endif