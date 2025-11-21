#define _POSIX_C_SOURCE 200809L  // <--- AÑADE ESTA LÍNEA AL PRINCIPIO
#include "memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>

void initListaMemoria(ListaMemoria *l) {
    l->data = NULL;
    l->tamano = 0;
    l->capacidad = 0;
}

void addBloqueMemoria(ListaMemoria *l, void *dir, size_t tam, TipoBloque tipo, key_t key, const char *fich, int fd) {
    if (l->tamano == l->capacidad) {
        int nuevaCapacidad = (l->capacidad == 0) ? 10 : l->capacidad * 2;
        BloqueMemoria *tmp = realloc(l->data, nuevaCapacidad * sizeof(BloqueMemoria));
        if (!tmp) {
            perror("No se pudo redimensionar la lista de memoria");
            return;
        }
        l->data = tmp;
        l->capacidad = nuevaCapacidad;
    }

    l->data[l->tamano].direccion = dir;
    l->data[l->tamano].tamano = tam;
    l->data[l->tamano].tiempo = time(NULL);
    l->data[l->tamano].tipo = tipo;

    if (tipo == SHARED) {
        l->data[l->tamano].info.key = key;
    } else if (tipo == MMAP) {
        l->data[l->tamano].info.mmap_data.nombre_fich = strdup(fich);
        l->data[l->tamano].info.mmap_data.fd = fd;
    }
    l->tamano++;
}

void freeListaMemoria(ListaMemoria *l) {
    for (int i = 0; i < l->tamano; i++) {
        if (l->data[i].tipo == MMAP) {
            free(l->data[i].info.mmap_data.nombre_fich);
        }
    }
    free(l->data);
    l->data = NULL;
    l->tamano = 0;
    l->capacidad = 0;
}
void printListaMemoria(const ListaMemoria *l, TipoBloque tipoFiltro) {
    for (int i = 0; i < l->tamano; i++) {
        if ((int)tipoFiltro != -1 && l->data[i].tipo != tipoFiltro) {
            continue;
        }

        BloqueMemoria *b = &l->data[i];
        struct tm *tm_info = localtime(&b->tiempo);
        char bufferTiempo[128];
        strftime(bufferTiempo, 26, "%b %d %H:%M", tm_info);
        printf("%20p%16lu %s ", b->direccion, b->tamano, bufferTiempo);

        if (b->tipo == MALLOC) {
            printf("malloc\n");
        } 
        else if (b->tipo == SHARED) {
            printf("shared (key %d)\n", b->info.key);
        } 
        else if (b->tipo == MMAP) {
            printf("mmap %s (fd:%d)\n", b->info.mmap_data.nombre_fich, b->info.mmap_data.fd);
        }
    }
}

void removeBloqueMemoria(ListaMemoria *l, void *dir) {
    int i;

    for (i = 0; i < l->tamano; i++) {
        if (l->data[i].direccion == dir) {

            if (l->data[i].tipo == MMAP) {
                free(l->data[i].info.mmap_data.nombre_fich);
            }

            for (int j = i; j < l->tamano - 1; j++) {
                l->data[j] = l->data[j + 1];
            }
            l->tamano--;
            return;
        }
    }
}