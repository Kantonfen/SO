#ifndef PROCESSLIST_H
#define PROCESSLIST_H

#include <sys/types.h>
#include <unistd.h>

typedef enum {
    ACTIVO,
    TERMINADO,
    DETENIDO,
    SENALADO
} EstadoProceso;

typedef struct {
    pid_t pid;
    char *comando;      
    char tiempo[32];    
    EstadoProceso estado;
    int info;           
} Proceso;

typedef struct {
    Proceso *data;      
    int tamano;
    int capacidad;
} ListaProcesos;


void initListaProcesos(ListaProcesos *l);
void freeListaProcesos(ListaProcesos *l);
void addProceso(ListaProcesos *l, pid_t pid, const char *cmd);
void removeProceso(ListaProcesos *l, pid_t pid);
void printListaProcesos(ListaProcesos *l);
void updateListaProcesos(ListaProcesos *l); 

#endif
