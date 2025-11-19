#include "openfiles.h"

void initOpenFiles(OpenFiles *l) {
    if (!l) return;
    l->capacidad = 10;                  //inicia con capacidad 10 y luego la duplica si se llena
    l->tamano = 0;
    l->data = malloc(l->capacidad * sizeof(ArchivoAbierto));
    if (!l->data) {
        perror("malloc openfiles");
        exit(EXIT_FAILURE);
    }
}

void freeOpenFiles(OpenFiles *l) {
    if (!l) return;
    for (int i = 0; i < l->tamano; i++) {
        free(l->data[i].nombre);
        free(l->data[i].modo);
    }
    free(l->data);
    l->data = NULL;
    l->tamano = 0;
    l->capacidad = 0;
}

void addOpenFile(OpenFiles *l, int fd, const char *nombre, const char *modo) {
    if (!l || !nombre || !modo) return;

    if (l->tamano == l->capacidad) {    //si esta lleno se duplica la capacidad
        int nueva = l->capacidad * 2;
        ArchivoAbierto *tmp = realloc(l->data, nueva * sizeof(ArchivoAbierto));
        if (!tmp) {
            perror("realloc openfiles");
            return;
        }
        l->data = tmp;              //se actualizan los valores
        l->capacidad = nueva;
    }

    l->data[l->tamano].fd = fd;                     //se guarda el nuevo archivo abierto
    l->data[l->tamano].nombre = strdup(nombre);
    l->data[l->tamano].modo = strdup(modo);
    if (!l->data[l->tamano].nombre || !l->data[l->tamano].modo) {
        perror("strdup openfiles");
        return;
    }
    l->tamano++;
}

void removeOpenFile(OpenFiles *l, int fd) {
    if (!l) return;
    for (int i = 0; i < l->tamano; i++) {               //busca el archivo por su fd
        if (l->data[i].fd == fd) {
            free(l->data[i].nombre);
            free(l->data[i].modo);
            
            for (int j = i; j < l->tamano - 1; j++) {  //compacta el array moviendo todos los archivos una pos atras
                l->data[j] = l->data[j + 1];
            }
            l->tamano--;
            return;
        }
    }
}

void printOpenFiles(const OpenFiles *l) {
    if (!l) return;
    if (l->tamano == 0) {
        printf("No hay archivos abiertos.\n");
        return;
    }
    for (int i = 0; i < l->tamano; i++) {
        printf("FD: %d  Nombre: %s  Modo: %s\n",
               l->data[i].fd,
               l->data[i].nombre,
               l->data[i].modo);
    }
}

ArchivoAbierto *findOpenFile(OpenFiles *l, int fd) {   
    if (!l) return NULL;
    for (int i = 0; i < l->tamano; i++) {   //busca un archivo por su fd y devuelve un puntero a su estructura
        if (l->data[i].fd == fd) {
            return &l->data[i];
        }
    }
    return NULL;
}