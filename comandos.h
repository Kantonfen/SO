#ifndef COMANDOS_H
#define COMANDOS_H

/* --- INCLUDES Y DEPENDENCIAS --- */
#include "historial.h"
#include "openfiles.h"
#include "memoria.h"
#include <sys/types.h>
#include <sys/stat.h>

/* --- ESTRUCTURAS DE CONFIGURACIÓN DE DIRECTORIOS --- */
typedef enum { SHORT, LONG } Formato;
typedef enum { NOLINK, LINK } LinkMode;
typedef enum { NOHID, HID } Hidden;
typedef enum { NOREC, RECB, RECA } Recursion;

typedef struct {
    Formato formato_largo;
    LinkMode mostrar_link;
    Hidden mostrar_ocultos;
    Recursion recursivo;
} DirParams;

// Variable global de configuración
extern DirParams dir_params;
extern char **environ; // Variable estándar de POSIX para el entorno


/* ==========================================================================
   SECCIÓN 1: INFORMACIÓN GENERAL DEL SHELL Y SISTEMA
   ========================================================================== */
void Cmd_authors(char *tr[]);
void Cmd_getpid(char *tr[]);
void Cmd_infosys(char *trozos[]);
void Cmd_help(char *tr[]);
void Cmd_date(char *tr[]);
void Cmd_hour(char *trozos[]);
void Cmd_historic(char *tr[], Historial *historial);


/* ==========================================================================
   SECCIÓN 2: GESTIÓN DE DIRECTORIOS Y SISTEMA DE FICHEROS
   ========================================================================== */
// Navegación y creación
void Cmd_chdir(char *tr[]);
void Cmd_getcwd(char *trozos[]);
void Cmd_create(char *tr[]);

// Configuración de listado (dir)
void Cmd_setdirparams(char *tr[]);
void Cmd_getdirparams(char *trozos[]);

// Listado y eliminación
void Cmd_dir(char *trozos[]);
void Cmd_dir_d(char *trozos[]);
void Cmd_erase(char *trozos[]);
void Cmd_delrec(char *trozos[]);

// Funciones auxiliares internas para directorios
char *ConvierteModo(mode_t m, char *permisos);
void ListarDirectorio(const char *path);
void RecorrerDirectorioRecursivo(const char *path);


/* ==========================================================================
   SECCIÓN 3: GESTIÓN DE ARCHIVOS ABIERTOS
   ========================================================================== */
// Operaciones sobre la tabla de archivos abiertos (OpenFiles)
void Cmd_open(char *tr[], OpenFiles *openFiles);
void Cmd_close(char *tr[], OpenFiles *openFiles);
void Cmd_dup(char *tr[], OpenFiles *openFiles);
void Cmd_listopen(char *tr[], OpenFiles *openFiles);

// Operaciones de E/S básicas sobre descriptores
void Cmd_writestr(char *tr[]);
void Cmd_lseek(char *tr[]);


/* ==========================================================================
   SECCIÓN 4: GESTIÓN DE MEMORIA - ASIGNACIÓN
   ========================================================================== */
// Comandos para reservar y liberar bloques
void Cmd_malloc(char *tr[], ListaMemoria *memoria);
void Cmd_shared(char *tr[], ListaMemoria *lm);
void Cmd_mmap(char *arg[], ListaMemoria *lm);
void Cmd_free(char *tr[], ListaMemoria *lm); // Liberación genérica


/* ==========================================================================
   SECCIÓN 5: MANIPULACIÓN DE DATOS EN MEMORIA
   ========================================================================== */
// Relleno y volcado de memoria
void Cmd_memfill(char *tr[], ListaMemoria *lm);
void Cmd_memdump(char *tr[], ListaMemoria *lm);

// Transferencia de datos (Archivo <-> Memoria)
void Cmd_readfile(char *tr[], ListaMemoria *lm);
void Cmd_writefile(char *tr[], ListaMemoria *lm);

// Lectura/Escritura usando descriptores de archivo (fd)
void Cmd_read(char *tr[], ListaMemoria *lm);
void Cmd_write(char *tr[], ListaMemoria *lm);


/* ==========================================================================
   SECCIÓN 6: INFORMACIÓN DEL PROCESO Y MEMORIA
   ========================================================================== */
void Cmd_recurse(char *tr[], ListaMemoria *lm); // Prueba de pila recursiva
void Cmd_mem(char *tr[], ListaMemoria *lm);     // Info de variables, funciones y pmap

void Cmd_uid(char *tr[]);
void Cmd_showenv(char *tr[], char *envp[]);
void Cmd_envvar(char *tr[], char *envp[]);

/* ==========================================================================
   SECCIÓN 7: NÚCLEO DEL PROCESAMIENTO
   ========================================================================== */
// Función principal que decide qué comando ejecutar
int ProcesarEntrada(char *entrada, Historial *historial, OpenFiles *openFiles, ListaMemoria *memoria , char *envp[]);




#endif