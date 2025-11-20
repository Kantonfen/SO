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

// Añade un bloque a la lista. 
// Nota: Pasamos todos los posibles parámetros. Si es MALLOC, 'key', 'fich' y 'fd' se ignoran.
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
    l->data[l->tamano].tiempo = time(NULL); // Guardamos el tiempo actual [cite: 14]
    l->data[l->tamano].tipo = tipo;

    // Guardamos info específica según el tipo [cite: 15, 16]
    if (tipo == SHARED) {
        l->data[l->tamano].info.key = key;
    } else if (tipo == MMAP) {
        l->data[l->tamano].info.mmap_data.nombre_fich = strdup(fich);
        l->data[l->tamano].info.mmap_data.fd = fd;
    }
    // En MALLOC no hay info extra que guardar en el union

    l->tamano++;
}

void freeListaMemoria(ListaMemoria *l) {
    for (int i = 0; i < l->tamano; i++) {
        // Si es MMAP, hay que liberar el strdup del nombre del fichero
        if (l->data[i].tipo == MMAP) {
            free(l->data[i].info.mmap_data.nombre_fich);
        }
        // Nota: Aquí NO liberamos el bloque de memoria (l->data[i].direccion)
        // Eso se hace con el comando free o malloc -free. 
        // Aquí solo limpiamos la lista de control del shell.
    }
    free(l->data);
    l->data = NULL;
    l->tamano = 0;
    l->capacidad = 0;
}

// Imprime la lista. Si el parámetro 'tipo' es -1, imprime todo.
// Si no, imprime solo los de ese tipo.
void printListaMemoria(const ListaMemoria *l, TipoBloque tipoFiltro) {
    for (int i = 0; i < l->tamano; i++) {
        // Filtrar si no pedimos imprimir todos (-1 se usará para "todos")
        if ((int)tipoFiltro != -1 && l->data[i].tipo != tipoFiltro) {
            continue;
        }

        BloqueMemoria *b = &l->data[i];
        struct tm *tm_info = localtime(&b->tiempo);
        char bufferTiempo[128];
        
        // Formato de tiempo: "Mon Nov 19 17:00:00 2025" aprox
        strftime(bufferTiempo, 26, "%b %d %H:%M", tm_info);

        // Imprimimos la parte común: dirección, tamaño, tiempo [cite: 11, 12, 13, 14]
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
    // Buscamos el bloque por su dirección
    for (i = 0; i < l->tamano; i++) {
        if (l->data[i].direccion == dir) {
            // Si es de tipo MMAP, hay que liberar la cadena duplicada del nombre
            if (l->data[i].tipo == MMAP) {
                free(l->data[i].info.mmap_data.nombre_fich);
            }
            // NOTA: NO hacemos free(l->data[i].direccion) aquí. 
            // Se asume que el comando que llama a esta función ya hizo el free del sistema 
            // o munmap correspondiente antes de pedir quitarlo de la lista.

            // Movemos los elementos siguientes una posición atrás para tapar el hueco
            for (int j = i; j < l->tamano - 1; j++) {
                l->data[j] = l->data[j + 1];
            }
            l->tamano--;
            return;
        }
    }
}