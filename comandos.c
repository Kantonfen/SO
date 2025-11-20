#define _POSIX_C_SOURCE 200809L

#include "comandos.h"
#include "openfiles.h"
#include "historial.h"
#include "memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/utsname.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>   // struct stat, lstat, S_IS*   para info de archivo y permisos
#include <unistd.h>     // unlink, rmdir, lseek, readlink, close, getpid, getppid, chdir, getcwd
#include <dirent.h>     // DIR, struct dirent, opendir, readdir, closedir
#include <limits.h>


DirParams dir_params = {SHORT, NOLINK, NOHID, NOREC};



int TrocearCadena(char *cadena, char *trozos[], int max_trozos) {
    int i = 0;
    char *tok;
    if (cadena == NULL || trozos == NULL || max_trozos <= 0) return 0;
    tok = strtok(cadena, " \n\t");
    while (tok != NULL && i < max_trozos - 1) {
        trozos[i++] = tok;
        tok = strtok(NULL, " \n\t");
    }
    trozos[i] = NULL;
    return i;
}

int ProcesarEntrada(char *entrada, Historial *historial, OpenFiles *openFiles, ListaMemoria *memoria) {
    char *trozos[64];
    int numPalabras;

    if (entrada == NULL) return 0;

    // 1. Guardar en historial (Movemos esto aquí para limpiar p1.c)
    // Eliminamos el salto de línea si existe para que quede bonito en el historial
    entrada[strcspn(entrada, "\n")] = '\0'; 
    addComando(historial, entrada);

    // 2. Trocear
    numPalabras = TrocearCadena(entrada, trozos, 64);
    if (numPalabras == 0) return 0; // Línea vacía, seguimos

    // 3. Procesar comandos
    if (strcmp(trozos[0], "quit") == 0 || strcmp(trozos[0], "exit") == 0 || strcmp(trozos[0], "bye") == 0) {
        return 1; // Indicamos que queremos SALIR
    }
    else if (strcmp(trozos[0], "authors") == 0) {
        //Cmd_authors(trozos);
    }
    else if (strcmp(trozos[0], "getpid") == 0) {
        //Cmd_getpid(trozos);
    }
    else if (strcmp(trozos[0], "chdir") == 0) {
        //Cmd_chdir(trozos);
    }
    else if (strcmp(trozos[0], "getcwd") == 0) {
        Cmd_getcwd(trozos);
    }
    else if (strcmp(trozos[0], "date") == 0) {
        //Cmd_date(trozos);
    }
    else if (strcmp(trozos[0], "hour") == 0) {
        //Cmd_hour(trozos);
    }
    else if (strcmp(trozos[0], "historic") == 0) {
        Cmd_historic(trozos, historial);
    }
    else if (strcmp(trozos[0], "infosys") == 0) {
        //Cmd_infosys(trozos);
    }
    else if (strcmp(trozos[0], "help") == 0) {
        //Cmd_help(trozos);
    }
    else if (strcmp(trozos[0], "create") == 0) {
        //Cmd_create(trozos);
    }
    else if (strcmp(trozos[0], "setdirparams") == 0) {
        //Cmd_setdirparams(trozos);
    }
    else if (strcmp(trozos[0], "getdirparams") == 0) {
        //Cmd_getdirparams(trozos);
    }
    else if (strcmp(trozos[0], "dir") == 0) {
        if (trozos[1] != NULL && strcmp(trozos[1], "-d") == 0) {
            //Cmd_dir_d(trozos);
        } else {
            //Cmd_dir(trozos);
        }
    }
    else if (strcmp(trozos[0], "erase") == 0) {
        //Cmd_erase(trozos);
    }
    else if (strcmp(trozos[0], "delrec") == 0) {
        //Cmd_delrec(trozos);
    }
    else if (strcmp(trozos[0], "open") == 0) {
        Cmd_open(trozos, openFiles);
    }
    else if (strcmp(trozos[0], "close") == 0) {
        Cmd_close(trozos, openFiles);
    }
    else if (strcmp(trozos[0], "dup") == 0) {
        Cmd_dup(trozos, openFiles);
    }
    else if (strcmp(trozos[0], "listopen") == 0) {
        Cmd_listopen(trozos, openFiles);
    }
    else if (strcmp(trozos[0], "writestr") == 0) {
        //Cmd_writestr(trozos);
    }
    else if (strcmp(trozos[0], "lseek") == 0) {
        //Cmd_lseek(trozos);
    }
    else if (strcmp(trozos[0], "malloc") == 0) {
        Cmd_malloc(trozos, memoria);
    }
    
    else {
        printf("Comando no reconocido: %s\n", trozos[0]);
    }

    return 0; // Continuamos ejecutando
}

void Cmd_chdir(char *tr[]){
    char cwd[PATH_MAX];

    if (tr == NULL || tr[0] == NULL) return;

    if (tr[1] == NULL) {
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("Directorio actual: %s\n", cwd);
        else
            perror("getcwd");
    } else {
        if (chdir(tr[1]) == -1) {
            perror(tr[1]);
        } else {
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("Directorio cambiado a: %s\n", cwd);
        }
    }
}

void Cmd_getcwd(char *trozos[]) {
    
    if (trozos != NULL && trozos[1] != NULL) {
        fprintf(stderr, "Uso: getcwd\n");
        return;
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Directorio actual: %s\n", cwd);
    } else {
        perror("getcwd");
    }
}

static int is_number(const char *s) {           //devuelve 1 si la cadena `s` representa un número entero válido (positivo o negativo), y 0 si no.
    if (s == NULL || *s == '\0') return 0;
    if (*s == '+' || *s == '-') s++;
    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

void Cmd_historic(char *tr[], Historial *historial){
    if (historial == NULL) return;

    if (tr == NULL || tr[0] == NULL || tr[1] == NULL) {   //imprime el historial
        printHistorial(historial);
        return;
    }

    if (strcmp(tr[1], "-count") == 0) {                   //cuenta cuantos comandos hay en historial
        printf("Hay %d comandos en la lista\n", historial->tamano);
        return;
    }

    if (strcmp(tr[1], "-clear") == 0) {                     //borra historial
        freeHistorial(historial);
        initHistorial(historial);
        printf("Historial borrado.\n");
        return;
    }

    
    if (is_number(tr[1]) && tr[1][0] != '-') {          //mostrar el comando n
        long n = strtol(tr[1], NULL, 10);
        
        if (n <= 0 || n > historial->tamano) {
            printf("Número de comando inválido: %ld\n", n);
            return;
        }
        
        printf("Comando %ld: %s\n", n, historial->data[n-1].comando);
        return;
    }

    
    if (is_number(tr[1]) && tr[1][0] == '-') {           //mostrar los últimos N comandos
        long n = strtol(tr[1], NULL, 10); 
        n = -n;
        if (n <= 0) {
            printf("Argumento inválido: %s\n", tr[1]);
            return;
        }
        if (n > historial->tamano) n = historial->tamano;
        int start = historial->tamano - (int)n;
        for (int i = start; i < historial->tamano; ++i) {
            printf("%d: %s\n", historial->data[i].num, historial->data[i].comando);
        }
        return;
    }

    printf("Uso: historic [N|-N|-count|-clear]\n");
}

void Cmd_open(char *tr[], OpenFiles *openFiles) {
    if (!openFiles) return;

    if (tr[1] == NULL) {                //Muéstrame la lista de archivos abiertos
        printOpenFiles(openFiles);
        return;
    }

    if (tr[2] == NULL) {
        fprintf(stderr, "Uso: open [archivo] [modo]\n");
        return;
    }

    const char *filename = tr[1];    //nombre del archivo
    const char *modo = tr[2];        //modo de apertura
    int flags = 0;                  //banderas del sistema(`O_RDONLY`, `O_CREAT`, etc.)

    if (strcmp(modo, "cr") == 0) flags = O_CREAT | O_RDWR;                      //crear el archivo si no existe y abrirlo para lectura/escritura.
    else if (strcmp(modo, "ap") == 0) flags = O_APPEND | O_WRONLY;              //abrir sólo para escritura y añadir siempre al final (append).
    else if (strcmp(modo, "ex") == 0) flags = O_EXCL | O_CREAT | O_WRONLY;      //crear exclusivamente; si ya existe el archivo
    else if (strcmp(modo, "ro") == 0) flags = O_RDONLY;                         //sólo lectura.
    else if (strcmp(modo, "rw") == 0) flags = O_RDWR;                           //
    else if (strcmp(modo, "wo") == 0) flags = O_WRONLY;                         //
    else if (strcmp(modo, "tr") == 0) flags = O_TRUNC | O_WRONLY;               //
    else {
        fprintf(stderr, "Modo desconocido: %s\n", modo);
        return;
    }

    int fd = open(filename, flags, 0666);                   // permisos por defecto `0666` (lectura y escritura para todos
    if (fd == -1) {
        perror(filename);
        return;
    }

    addOpenFile(openFiles, fd, filename, modo);
    printf("Archivo '%s' abierto con descriptor %d\n", filename, fd);
}

void Cmd_close(char *tr[], OpenFiles *openFiles) {
    if (!openFiles) return;

    if (tr[1] == NULL) {
        fprintf(stderr, "Uso: close [df]\n");
        return;
    }

    int fd = atoi(tr[1]);
    if (close(fd) == -1) {
        perror("close");
        return;
    }

    removeOpenFile(openFiles, fd);
    printf("Descriptor %d cerrado correctamente.\n", fd);
}

void Cmd_dup(char *tr[], OpenFiles *openFiles) {
    if (!openFiles) return;

    if (tr[1] == NULL) {
        fprintf(stderr, "Uso: dup [df]\n");
        return;
    }

    int fd = atoi(tr[1]);
    ArchivoAbierto *orig = findOpenFile(openFiles, fd);
    if (!orig) {
        fprintf(stderr, "Descriptor %d no encontrado en la lista.\n", fd);
        return;
    }

    int newfd = dup(fd);
    if (newfd == -1) {
        perror("dup");
        return;
    }

    addOpenFile(openFiles, newfd, orig->nombre, orig->modo);
    printf("Descriptor %d duplicado como %d.\n", fd, newfd);
}

void Cmd_listopen(char *tr[], OpenFiles *openFiles) {
    (void)tr; // evitar warning
    printOpenFiles(openFiles);
}


void Cmd_malloc(char *tr[], ListaMemoria *lm) {
    // CASO 1: Listar bloques malloc (sin argumentos) 
    if (tr[1] == NULL) {
        printf("******Lista de bloques asignados malloc para el proceso %d\n", getpid());
        printListaMemoria(lm, MALLOC);
        return;
    }

    // CASO 2: Liberar (malloc -free n) - DE MOMENTO PENDIENTE
    if (strcmp(tr[1], "-free") == 0) {
        // if (tr[2] == NULL) ... error
        printf("Funcionalidad de liberar pendiente de implementar...\n");
        return;
    }

    // CASO 3: Asignar memoria (malloc n) 
    // Convertimos el argumento a numero
    size_t tam = (size_t)strtoul(tr[1], NULL, 10);
    
    if (tam == 0) {
        printf("No se asignan bloques de 0 bytes\n");
        return;
    }

    // 1. Hacer el malloc del sistema
    void *dir = malloc(tam);
    if (dir == NULL) {
        perror("Error al asignar memoria");
        return;
    }

    // 2. Añadir a nuestra lista de control
    // Pasamos 0 y NULL en los campos de shared y mmap porque no aplican
    addBloqueMemoria(lm, dir, tam, MALLOC, 0, NULL, 0); 

    printf("Asignados %lu bytes en %p\n", tam, dir);
}