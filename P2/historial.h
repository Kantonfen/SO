#ifndef HISTORIAL_H
#define HISTORIAL_H


#include <stdio.h>
#include <stdlib.h>


typedef struct {
    int num;            // numero de orden del comando
    char *comando;      //cadena de texto
} Comando;

typedef struct {
    Comando *data;      //array dinamico de Comando
    int tamano;         //cuantos comandos hay almacenados
    int capacidad;      //cantidad de memoria reservada
} Historial;

void initHistorial(Historial *h);
void addComando(Historial *h, const char *cmd);
void printHistorial(const Historial *h);
void freeHistorial(Historial *h);

#endif