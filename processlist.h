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
    char *comando;      // Línea de comando ejecutada
    char tiempo[32];    // Fecha y hora de lanzamiento
    EstadoProceso estado;
    int info;           // Valor de retorno (si TERMINADO) o señal (si SENALADO/DETENIDO)
} Proceso;

typedef struct {
    Proceso *data;      // Array dinámico
    int tamano;
    int capacidad;
} ListaProcesos;

// Funciones de gestión de la lista
void initListaProcesos(ListaProcesos *l);
void freeListaProcesos(ListaProcesos *l);
void addProceso(ListaProcesos *l, pid_t pid, const char *cmd);
void removeProceso(ListaProcesos *l, pid_t pid);
void printListaProcesos(ListaProcesos *l);
void updateListaProcesos(ListaProcesos *l); // Actualiza el estado con waitpid

#endif
