#define _POSIX_C_SOURCE 200809L


#include "historial.h"
#include <string.h>


void initHistorial(Historial *h){
    if (!h) return;
    h->capacidad = 10;
    h->tamano = 0;
    h->data = malloc(h->capacidad * sizeof(Comando)); //reserva memoria para 10 y se duplica cuando llena
    if (h->data == NULL) {
        perror("malloc historial");
        exit(EXIT_FAILURE);
    }
}

void addComando(Historial *h, const char *cmd){
    if (!h || !cmd) return;

    if (h->tamano == h->capacidad){
        int nueva = h->capacidad * 2;
        Comando *tmp = realloc(h->data, nueva * sizeof(Comando));  //amplia el bloque de mem y conserva los datos
        if (tmp == NULL) {
            perror("realloc historial");
            return;
        }
        h->data = tmp;                  //actualiza data y capacidad al nuevo tamaño
        h->capacidad = nueva;
    }

    //guarda el nuevo comando
    h->data[h->tamano].num = h->tamano + 1;    //empieza desde 1
    h->data[h->tamano].comando = strdup(cmd);
    if (h->data[h->tamano].comando == NULL) {
        perror("strdup comando");
        return;
    }
    h->tamano++;
}

void printHistorial(const Historial *h){
    if (!h) return;
    for (int i = 0; i < h->tamano; i++){
        printf("%d: %s\n", h->data[i].num, h->data[i].comando);
    }
}

void freeHistorial(Historial *h){
    if (!h) return;
    for (int i = 0; i < h->tamano; i++){
        free(h->data[i].comando);                   //libera cada comando
    }
    free(h->data);                                  //luego el array entero
    h->data = NULL;                                 //puntero a Null y contadores a 0
    h->tamano = 0;
    h->capacidad = 0;
}
