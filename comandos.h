#ifndef COMANDOS_H
#define COMANDOS_H

#include "historial.h"
#include "openfiles.h"
#include "memoria.h"
#include <sys/types.h>
#include <sys/stat.h>

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

extern DirParams dir_params;

char *ConvierteModo(mode_t m, char *permisos);
void Cmd_create(char *tr[]);
void Cmd_setdirparams(char *tr[]);
void Cmd_getdirparams(char *trozos[]);
void Cmd_authors(char *tr[]);
void Cmd_getpid(char *tr[]);
void Cmd_chdir(char *tr[]);
void Cmd_getcwd(char *trozos[]);
void Cmd_date(char *tr[]);
void Cmd_hour(char *trozos[]);
void Cmd_historic(char *tr[], Historial *historial);
void Cmd_infosys(char *trozos[]);
void Cmd_help(char *tr[]);

void ListarDirectorio(const char *path);
void RecorrerDirectorioRecursivo(const char *path);
void Cmd_dir(char *trozos[]);
void Cmd_dir_d(char *trozos[]);
void Cmd_erase(char *trozos[]);
void Cmd_delrec(char *trozos[]);

void Cmd_open(char *tr[], OpenFiles *openFiles);
void Cmd_close(char *tr[], OpenFiles *openFiles);
void Cmd_dup(char *tr[], OpenFiles *openFiles);
void Cmd_listopen(char *tr[], OpenFiles *openFiles);

void Cmd_writestr(char *tr[]);
void Cmd_lseek(char *tr[]);

int ProcesarEntrada(char *entrada, Historial *historial, OpenFiles *openFiles, ListaMemoria *memoria);

#endif