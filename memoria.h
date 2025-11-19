#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>

typedef enum { MALLOC, SHARED, MMAP } TipoBloque;

typedef struct {
    void *direccion;
    size_t tamano;
    time_t tiempo;
    TipoBloque tipo;
    union {
        key_t key;
        struct {
            char *nombre_fich;
            int fd;
        } mmap_data;
    } info;
} BloqueMemoria;

typedef struct {
    BloqueMemoria *data;
    int tamano;
    int capacidad;
} ListaMemoria;

// Funciones básicas (las implementaremos pronto)
void initListaMemoria(ListaMemoria *l);
void freeListaMemoria(ListaMemoria *l);
// void addBloqueMemoria(...); // Lo añadiremos luego para no complicarnos ahora
void printListaMemoria(const ListaMemoria *l);

#endif