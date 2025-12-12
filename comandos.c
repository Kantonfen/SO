#define _GNU_SOURCE

#include "comandos.h"
#include "openfiles.h"
#include "historial.h"
#include "memoria.h"
#include "processlist.h"
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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pwd.h>
#include <sys/resource.h>



DirParams dir_params = {SHORT, NOLINK, NOHID, NOREC};

// VARIABLES GLOBALES PARA 'mem -vars'
int v_global1 = 10, v_global2 = 20, v_global3 = 30;     // Externas inicializadas
int v_global_u1, v_global_u2, v_global_u3;              // Externas no inicializadas


/* ==========================================================================
   SECCIÓN 0: Procesamiento de entrada y Funciones auxiliares
   ========================================================================== */


typedef struct {
    int background;
    int priority;
    char **argv_exec; 
} InfoEjecucion;

InfoEjecucion AnalizarProgSpec(char *tr[]) {
    InfoEjecucion info;
    info.background = 0;
    info.priority = 0;
    int hay_priority = 0;
    info.argv_exec = tr;

    int i = 0;
    while (tr[i] != NULL) i++; 
    if (i == 0) return info;


    if (strcmp(tr[i-1], "&") == 0) {
        info.background = 1;
        tr[i-1] = NULL;
        i--; 
    }


    for (int j = 0; j < i; j++) {
        if (tr[j][0] == '@') {
            info.priority = atoi(tr[j] + 1); 
            hay_priority = 1;
            tr[j] = NULL;

            break; 
        }
    }
    if (!hay_priority) info.priority = -9999;

    return info;
}


char *ReconstruirComando(char *argv[]) {
    static char buffer[1024];
    buffer[0] = '\0';
    
    for (int i = 0; argv[i] != NULL; i++) {
        strcat(buffer, argv[i]);
        if (argv[i+1] != NULL) strcat(buffer, " "); 
    }
    return buffer;
}

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

int ProcesarEntrada(char *entrada, Historial *historial, OpenFiles *openFiles, ListaMemoria *memoria , char *envp[], ListaProcesos *procesos) {
    char *trozos[64];
    int numPalabras;

    if (entrada == NULL) return 0;

    entrada[strcspn(entrada, "\n")] = '\0'; 
    addComando(historial, entrada);


    numPalabras = TrocearCadena(entrada, trozos, 64);
    if (numPalabras == 0) return 0;


    if (strcmp(trozos[0], "quit") == 0 || strcmp(trozos[0], "exit") == 0 || strcmp(trozos[0], "bye") == 0) {
        return 1;
    }
    else if (strcmp(trozos[0], "authors") == 0) {
        Cmd_authors(trozos);
    }
    else if (strcmp(trozos[0], "getpid") == 0) {
        Cmd_getpid(trozos);
    }
    else if (strcmp(trozos[0], "chdir") == 0) {
        Cmd_chdir(trozos);
    }
    else if (strcmp(trozos[0], "getcwd") == 0) {
        Cmd_getcwd(trozos);
    }
    else if (strcmp(trozos[0], "date") == 0) {
        Cmd_date(trozos);
    }
    else if (strcmp(trozos[0], "hour") == 0) {
        Cmd_hour(trozos);
    }
    else if (strcmp(trozos[0], "historic") == 0) {
        Cmd_historic(trozos, historial);
    }
    else if (strcmp(trozos[0], "infosys") == 0) {
        Cmd_infosys(trozos);
    }
    else if (strcmp(trozos[0], "help") == 0) {
        Cmd_help(trozos);
    }
    else if (strcmp(trozos[0], "create") == 0) {
        Cmd_create(trozos);
    }
    else if (strcmp(trozos[0], "setdirparams") == 0) {
        Cmd_setdirparams(trozos);
    }
    else if (strcmp(trozos[0], "getdirparams") == 0) {
        Cmd_getdirparams(trozos);
    }
    else if (strcmp(trozos[0], "dir") == 0) {
        if (trozos[1] != NULL && strcmp(trozos[1], "-d") == 0) {
            Cmd_dir_d(trozos);
        } else {
            Cmd_dir(trozos);
        }
    }
    else if (strcmp(trozos[0], "erase") == 0) {
        Cmd_erase(trozos);
    }
    else if (strcmp(trozos[0], "delrec") == 0) {
        Cmd_delrec(trozos);
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
        Cmd_writestr(trozos);
    }
    else if (strcmp(trozos[0], "lseek") == 0) {
        Cmd_lseek(trozos);
    }
    else if (strcmp(trozos[0], "malloc") == 0) {
        Cmd_malloc(trozos, memoria);
    }
    else if (strcmp(trozos[0], "shared") == 0) {
        Cmd_shared(trozos, memoria);
    }
    else if (strcmp(trozos[0], "mmap") == 0) {
        Cmd_mmap(trozos, memoria);
    }
    else if (strcmp(trozos[0], "free") == 0) {
        Cmd_free(trozos, memoria);
    }
    else if (strcmp(trozos[0], "memfill") == 0) {
        Cmd_memfill(trozos, memoria);
    }
    else if (strcmp(trozos[0], "memdump") == 0) {
        Cmd_memdump(trozos, memoria);
    }
    else if (strcmp(trozos[0], "readfile") == 0) {
        Cmd_readfile(trozos, memoria);
    }
    else if (strcmp(trozos[0], "writefile") == 0) {
        Cmd_writefile(trozos, memoria);
    }
    else if (strcmp(trozos[0], "read") == 0) {
        Cmd_read(trozos, memoria);
    }
    else if (strcmp(trozos[0], "write") == 0) {
        Cmd_write(trozos, memoria);
    }
    else if (strcmp(trozos[0], "recurse") == 0) {
    Cmd_recurse(trozos, memoria);
    }
    else if (strcmp(trozos[0], "mem") == 0) {
    Cmd_mem(trozos, memoria);
    }
    else if (strcmp(trozos[0], "uid") == 0) {
    Cmd_uid(trozos);
    }
    else if (strcmp(trozos[0], "showenv") == 0) {
        Cmd_showenv(trozos, envp);
    }
    else if (strcmp(trozos[0], "envvar") == 0) {
        Cmd_envvar(trozos, envp);
    }
    else if (strcmp(trozos[0], "fork") == 0) {
        Cmd_fork(trozos);
    }
    else if (strcmp(trozos[0], "jobs") == 0) {
        Cmd_jobs(trozos, procesos);
    }
    else if (strcmp(trozos[0], "deljobs") == 0) {
        Cmd_deljobs(trozos, procesos);
    }
    else if (strcmp(trozos[0], "exec") == 0) {
        Cmd_exec(trozos);
    }
 else {
        Cmd_lanzar(trozos, procesos);
    }
    return 0;
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

char *ConvierteModo(mode_t m, char *permisos) {
    
    strcpy(permisos, "----------");

    if (S_ISDIR(m)) permisos[0] = 'd';
    if (S_ISLNK(m)) permisos[0] = 'l';
    if (S_ISCHR(m)) permisos[0] = 'c';
    if (S_ISBLK(m)) permisos[0] = 'b';
    if (S_ISFIFO(m)) permisos[0] = 'p';
    if (S_ISSOCK(m)) permisos[0] = 's';

    if (m & S_IRUSR) permisos[1] = 'r';
    if (m & S_IWUSR) permisos[2] = 'w';
    if (m & S_IXUSR) permisos[3] = 'x';
    if (m & S_IRGRP) permisos[4] = 'r';
    if (m & S_IWGRP) permisos[5] = 'w';
    if (m & S_IXGRP) permisos[6] = 'x';
    if (m & S_IROTH) permisos[7] = 'r';
    if (m & S_IWOTH) permisos[8] = 'w';
    if (m & S_IXOTH) permisos[9] = 'x';

    permisos[10] = '\0';
    return permisos;
}

void ImprimeEntrada(const char *ruta, const struct stat *st) {
    char permisos[12];
    if (dir_params.formato_largo == LONG) {
        printf("%s %lld %s\n",
               ConvierteModo(st->st_mode, permisos),
               (long long)st->st_size,
               ruta);
    } else {
        printf("%s\n", ruta);
    }
}

void RecorrerDirectorioRecursivo(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    if (!S_ISDIR(st.st_mode)) {
        ImprimeEntrada(path, &st);
        return;
    }

    
    if (dir_params.recursivo == RECB) {
        ImprimeEntrada(path, &st); 
    } else if (dir_params.recursivo == NOREC) {
        ImprimeEntrada(path, &st); 
    }

    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);
        return;
    }

    struct dirent *entry;
    char ruta[PATH_MAX];
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (dir_params.mostrar_ocultos == NOHID && entry->d_name[0] == '.') continue;

        if (snprintf(ruta, sizeof(ruta), "%s/%s", path, entry->d_name) >= (int)sizeof(ruta)) {
            fprintf(stderr, "Ruta demasiado larga: %s/%s\n", path, entry->d_name);
            continue;
        }

        if (dir_params.recursivo == NOREC) {
            struct stat st2;
            if (lstat(ruta, &st2) == -1) {
                perror(ruta);
            } else {
                ImprimeEntrada(ruta, &st2);
            }
        } else {
            RecorrerDirectorioRecursivo(ruta);
        }
    }
    closedir(dir);

    if (dir_params.recursivo == RECA) {     //el directorio se imprime después de listar su contenido
        if (lstat(path, &st) == -1) {
            perror(path);
        } else {
            ImprimeEntrada(path, &st);
        }
    }
}

int borrar_recursivo(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror(path);
        return -1;
    }
    if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode)) {
        if (unlink(path) == -1) {           //`unlink` elimina **archivos** pero no directorios.
            perror(path);
            return -1;
        }
        return 0;
    }
    if (S_ISDIR(st.st_mode)) {      //Si es un **directorio**, se abre con `opendir`.
        DIR *dir = opendir(path);
        if (!dir) {
            perror(path);
            return -1;
        }
        struct dirent *entry;
        char ruta[PATH_MAX];
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            if (snprintf(ruta, sizeof(ruta), "%s/%s", path, entry->d_name) >= (int)sizeof(ruta)) {
                fprintf(stderr, "Ruta demasiado larga: %s/%s\n", path, entry->d_name);
                continue;
            }
            if (borrar_recursivo(ruta) == -1) {
                /* seguimos intentando borrar otros ficheros */
            }
        }
        closedir(dir);
        if (rmdir(path) == -1) {
            perror(path);
            return -1;
        }
        return 0;
    }
    return 0;
}

void ListarDirectorio(const char *path) {
    DIR *dir = opendir(path);               //acceder al directorio.
    if (!dir) {
        perror(path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        
        if (dir_params.mostrar_ocultos == NOHID && entry->d_name[0] == '.')             //se salta los ocultos
            continue;

        
        char ruta[PATH_MAX];
        if (snprintf(ruta, sizeof(ruta), "%s/%s", path, entry->d_name) >= (int)sizeof(ruta)) {              //consrtuye la ruta
            fprintf(stderr, "Ruta demasiado larga: %s/%s\n", path, entry->d_name);
            continue;
        }

        struct stat st;                         
        if (lstat(ruta, &st) == -1) {               //Usa `lstat()` en lugar de `stat()` para no seguir enlaces simbólicos  
            perror(ruta);
            continue;
        }

        
        char nombre_mostrar[PATH_MAX * 2 + 32];
        if (dir_params.mostrar_link == LINK && S_ISLNK(st.st_mode)) {
            char link_dest[PATH_MAX];
            ssize_t len = readlink(ruta, link_dest, sizeof(link_dest) - 1);             //crea la ruta con formato mi_link -> /usr/bin/gcv
            if (len != -1) {
                link_dest[len] = '\0';
                int n = snprintf(nombre_mostrar, sizeof(nombre_mostrar), "%s -> %s", entry->d_name, link_dest);
                if (n < 0 || n >= (int)sizeof(nombre_mostrar)) {
                    nombre_mostrar[sizeof(nombre_mostrar) - 1] = '\0';
                }
            } else {
                strncpy(nombre_mostrar, entry->d_name, sizeof(nombre_mostrar) - 1);     //si readlink devuelve el nombre del archivo
                nombre_mostrar[sizeof(nombre_mostrar) - 1] = '\0';
            }
        } else {
            strncpy(nombre_mostrar, entry->d_name, sizeof(nombre_mostrar) - 1);         //si no es un enlace simbolico se copia el nombre normal
            nombre_mostrar[sizeof(nombre_mostrar) - 1] = '\0';
        }

        
        if (dir_params.formato_largo == LONG) {             //muestra permisos tamaño nombre 
            char permisos[12];
            printf("%s %10lld %s\n",
                   ConvierteModo(st.st_mode, permisos),
                   (long long)st.st_size,
                   nombre_mostrar);
        } else {
            printf("%-30s %10lld\n", nombre_mostrar, (long long)st.st_size);  //muestra tamaño y nombre
        }
    }

    closedir(dir);
}

void *CadenaToPointer(char *s) {
    void *p;
    sscanf(s, "%p", &p);
    return p;
}

void *ObtenerMemoriaShmget(key_t clave, size_t tam, ListaMemoria *lm) {
    void *p;
    int aux, id, flags = 0777;
    struct shmid_ds s;

    if (tam)
        flags = flags | IPC_CREAT | IPC_EXCL;

    if (clave == IPC_PRIVATE) {
        errno = EINVAL;
        return NULL;
    }

    if ((id = shmget(clave, tam, flags)) == -1)
        return (NULL);

    if ((p = shmat(id, NULL, 0)) == (void *)-1) {
        aux = errno;
        if (tam)
            shmctl(id, IPC_RMID, NULL);
        errno = aux;
        return (NULL);
    }

    shmctl(id, IPC_STAT, &s);

    addBloqueMemoria(lm, p, s.shm_segsz, SHARED, clave, NULL, 0);

    return (p);
}

void *MapearFichero(char *fichero, int protection, ListaMemoria *lm) {
    int df, map = MAP_PRIVATE, modo = O_RDONLY;
    struct stat s;
    void *p;

    if (protection & PROT_WRITE)
        modo = O_RDWR;

    if (stat(fichero, &s) == -1 || (df = open(fichero, modo)) == -1)
        return NULL;

    if ((p = mmap(NULL, s.st_size, protection, map, df, 0)) == MAP_FAILED){
        close(df);
        return NULL;
    }
    addBloqueMemoria(lm, p, s.st_size, MMAP, 0, fichero, df);

    return p;
}

void LlenarMemoria (void *p, size_t cont, unsigned char byte)
{
  unsigned char *arr=(unsigned char *) p;
  size_t i;

  for (i=0; i<cont;i++)
        arr[i]=byte;
}

ssize_t LeerFichero(char *f, void *p, size_t cont) {
    struct stat s;
    ssize_t n;
    int df, aux;

    if (stat(f, &s) == -1 || (df = open(f, O_RDONLY)) == -1)
        return -1;

    if (cont == (size_t)-1)
        cont = s.st_size;

    if ((n = read(df, p, cont)) == -1) {
        aux = errno;
        close(df);
        errno = aux;
        return -1;
    }
    close(df);
    return n;
}

ssize_t EscribirFichero(char *f, void *p, size_t cont) {
    ssize_t n;
    int df, aux;

    if ((df = open(f, O_CREAT | O_WRONLY | O_TRUNC, 0666)) == -1)
        return -1;

    if ((n = write(df, p, cont)) == -1) {
        aux = errno;
        close(df);
        errno = aux;
        return -1;
    }
    close(df);
    return n;
}

void Recursiva(int n) {
    char automatico[1024];
    static char estatico[1024];

    printf("parametro:%3d(%p) array %p, arr estatico %p\n", n, &n, automatico, estatico);

    if (n > 0)
        Recursiva(n - 1);
}

void Do_pmap(void) {
    pid_t pid;
    char elpid[32];
    char *argv[4] = {"pmap", elpid, NULL};

    sprintf(elpid, "%d", (int)getpid());
    if ((pid = fork()) == -1) {
        perror("Imposible crear proceso");
        return;
    }
    if (pid == 0) {
        if (execvp(argv[0], argv) == -1) {
            perror("cannot execute pmap (linux, solaris)");
            argv[0] = "procstat"; argv[1] = "vm"; argv[2] = elpid; argv[3] = NULL;
            if (execvp(argv[0], argv) == -1) {
                 perror("cannot execute procstat (FreeBSD)");
                 argv[0] = "procmap"; argv[1] = elpid; argv[2] = NULL;
                 if (execvp(argv[0], argv) == -1) {
                      perror("cannot execute procmap (OpenBSD)");
                      argv[0] = "vmmap"; argv[1] = "-interleave"; argv[2] = elpid; argv[3] = NULL;
                      if (execvp(argv[0], argv) == -1) {
                          perror("cannot execute vmmap (Mac-OS)");
                      }
                 }
            }
        }
        exit(1);
    }
    waitpid(pid, NULL, 0);
}

void MostrarCredenciales() {
    uid_t real = getuid();
    struct passwd *pw_real = getpwuid(real);
    char *nombre_real = (pw_real) ? strdup(pw_real->pw_name) : strdup("???");

    uid_t efect = geteuid();
    struct passwd *pw_efect = getpwuid(efect);

    printf("Credencial Real: %d, (%s)\n", real, nombre_real);
    printf("Credencial Efectiva: %d, (%s)\n", efect, (pw_efect ? pw_efect->pw_name : "???"));

    free(nombre_real);
}

#define MAXVAR 1024 // Asegúrate de definir esto si no está

int BuscarVariable(char * var, char *e[]) {
  int pos = 0;
  char aux[MAXVAR];
  
  strcpy(aux, var);
  strcat(aux, "=");
  
  while (e[pos] != NULL)
    if (!strncmp(e[pos], aux, strlen(aux)))
      return (pos);
    else 
      pos++;
  errno = ENOENT;
  return(-1);
}

int CambiarVariable(char * var, char * valor, char *e[]) {
  int pos;
  char *aux;
   
  if ((pos = BuscarVariable(var, e)) == -1)
    return(-1);
 
  if ((aux = (char *)malloc(strlen(var) + strlen(valor) + 2)) == NULL)
    return -1;
  strcpy(aux, var);
  strcat(aux, "=");
  strcat(aux, valor);
  e[pos] = aux;
  return (pos);
}
/* ==========================================================================
   SECCIÓN 1: INFORMACIÓN GENERAL DEL SHELL Y SISTEMA
   ========================================================================== */

void Cmd_authors(char *tr[]){
    
    if (tr == NULL || tr[0] == NULL) return;

    if (tr[1] == NULL) {
        printf("Autores:\n  Daniel Cabrera Herrera (daniel.cabrera)\n  Xoel Queiro Lema (xoel.queiro)\n");
    } else if (strcmp(tr[1], "-l") == 0) {
        printf("Logins:\n  daniel.cabrera\n  xoel.queiro\n");
    } else if (strcmp(tr[1], "-n") == 0) {
        printf("Nombres:\n  Daniel Cabrera Herrera\n  Xoel Queiro Lema\n");
    } else {
        printf("Uso: authors [-l|-n]\n");
    }
}

void Cmd_getpid(char *tr[]){
    pid_t pid = getpid();  //proceso actual (tu shell)
    pid_t ppid = getppid(); //proceso padre (el que lanzo tu shell)

    if (tr == NULL || tr[0] == NULL) return;

    if (tr[1] == NULL) {
        printf("PID: %d\n", (int)pid);
    } else if (strcmp(tr[1], "-p") == 0) {
        printf("PID padre: %d\n", (int)ppid);
    } else {
        printf("Uso: getpid [-p]\n");
    }
}

void Cmd_infosys(char *trozos[]) {
    
    if (trozos != NULL && trozos[1] != NULL) {
        fprintf(stderr, "Uso: infosys\n");
        return;
    }

    struct utsname info;
    if (uname(&info) == -1) {
        perror("uname");
        return;
    }
    printf("Sistema: %s\n", info.sysname);
    printf("Nodo:    %s\n", info.nodename);
    printf("Release: %s\n", info.release);
    printf("Version: %s\n", info.version);
    printf("Máquina: %s\n", info.machine);
}

void Cmd_help(char *tr[]){
    if (tr == NULL || tr[0] == NULL) return;

    
    if (tr[1] == NULL) {
        printf("Comandos disponibles:\n");
        printf("  authors [-l|-n]            : Mostrar autores (nombres/logins).\n");
        printf("  getpid [-p]                : Mostrar PID del shell (o PID padre con -p).\n");
        printf("  chdir [dir]                : Cambiar directorio o mostrar el actual.\n");
        printf("  getcwd                     : Mostrar el directorio de trabajo actual.\n");
        printf("  date [-d|-t]               : Fecha y hora (DD/MM/YYYY HH:MM:SS).\n");
        printf("  hour                       : Mostrar solo la hora (HH:MM:SS).\n");
        printf("  historic [N|-N|-count|-clear]: Histórico de comandos.\n");
        printf("  create [-f] nam            : Crear directorio (o fichero con -f).\n");
        printf("  setdirparams <opción>      : Ajustar parámetros de listado (ver ayuda).\n");
        printf("  getdirparams               : Mostrar parámetros actuales de 'dir'.\n");
        printf("  dir [-d] [n1 n2 ...]       : Listar ficheros/directorios (ver opciones).\n");
        printf("  erase n1 n2 ...            : Borrar ficheros o directorios VACÍOS.\n");
        printf("  delrec n1 n2 ...           : Borrar recursivamente ficheros/directorios.\n");
        printf("  open [archivo] modo        : Abrir archivo (sin args lista abiertos).\n");
        printf("  close df                   : Cerrar descriptor de archivo.\n");
        printf("  dup df                     : Duplicar descriptor de archivo.\n");
        printf("  listopen                   : Listar archivos abiertos por el shell.\n");
        printf("  writestr df \"str\"         : Escribir cadena 'str' en descriptor df.\n");
        printf("  lseek df off ref           : Cambiar offset (SEEK_SET|SEEK_CUR|SEEK_END).\n");
        printf("  infosys                    : Mostrar información del sistema (uname).\n");
        printf("  help [cmd]                 : Mostrar esta ayuda o la de 'cmd'.\n");
        printf("  quit | exit | bye          : Salir del shell.\n");
        return;
    }

    
    if (strcmp(tr[1], "authors") == 0) {
        printf("authors [-l|-n]\n");
        printf("  - Sin argumentos: muestra nombres y logins de los autores.\n");
        printf("  - -l : muestra sólo los logins.\n");
        printf("  - -n : muestra sólo los nombres.\n");
        printf("  Ej: authors -l\n");
        return;
    }

    if (strcmp(tr[1], "getpid") == 0) {
        printf("getpid [-p]\n");
        printf("  - Sin args: muestra el PID del proceso del shell.\n");
        printf("  - -p : muestra el PID del proceso padre.\n");
        return;
    }

    if (strcmp(tr[1], "chdir") == 0) {
        printf("chdir [dir]\n");
        printf("  - Sin argumentos: muestra el directorio actual.\n");
        printf("  - chdir dir : cambia el directorio actual a 'dir' (usa chdir(2)).\n");
        return;
    }

    if (strcmp(tr[1], "getcwd") == 0) {
        printf("getcwd\n");
        printf("  Muestra el directorio de trabajo actual (getcwd).\n");
        return;
    }

    if (strcmp(tr[1], "date") == 0) {
        printf("date [-d|-t]\n");
        printf("  - Sin args: muestra fecha y hora en formato DD/MM/YYYY HH:MM:SS.\n");
        printf("  - -d : solo fecha (DD/MM/YYYY).\n");
        printf("  - -t : solo hora (HH:MM:SS).\n        ");
        return;
    }

    if (strcmp(tr[1], "hour") == 0) {
        printf("hour\n");
        printf("  Muestra solo la hora (equivalente a 'date -t').\n");
        return;
    }

    if (strcmp(tr[1], "historic") == 0) {
        printf("historic [N|-N|-count|-clear]\n");
        printf("  - Sin argumentos: muestra todo el histórico numerado.\n");
        printf("  - historic N   : muestra (o repite visualmente) el comando N (numeración desde 1).\n");
        printf("  - historic -N  : muestra los últimos N comandos.\n");
        printf("  - historic -count : muestra cuántos comandos hay en el histórico.\n");
        printf("  - historic -clear : borra el histórico.\n");
        return;
    }

    if (strcmp(tr[1], "create") == 0) {
        printf("create [-f] nam\n");
        printf("  - create nam      : crea un directorio llamado 'nam'.\n");
        printf("  - create -f nam   : crea un fichero vacío llamado 'nam'.\n");
        printf("  Ej: create -f prueba.txt\n");
        return;
    }

    if (strcmp(tr[1], "setdirparams") == 0) {
        printf("setdirparams <opción>\n");
        printf("  Opciones posibles:\n");
        printf("   - long | short     : formato largo (permisos/tamaño) o corto (solo nombre).\n");
        printf("   - link | nolink    : mostrar destino de enlaces simbólicos o no.\n");
        printf("   - hid | nohid      : mostrar archivos ocultos o no.\n");
        printf("   - reca | recb | norec : recursión (reca: post-order, recb: pre-order, norec: no recursión).\n");
        printf("  Ej: setdirparams long\n");
        return;
    }

    if (strcmp(tr[1], "getdirparams") == 0) {
        printf("getdirparams\n");
        printf("  Muestra los parámetros actuales usados por 'dir'.\n");
        return;
    }

    if (strcmp(tr[1], "dir") == 0) {
        printf("dir [-d] [n1 n2 ...]\n");
        printf("  - dir [n1 n2 ...] : lista información de los nombres indicados (o '.' si vacio).\n");
        printf("  - dir -d [n1 ...] : si alguna entrada es un directorio, lista su contenido (con opciones de recursión/hid/link).\n");
        printf("  El comportamiento de formato/ocultos/recursión se controla con setdirparams.\n");
        return;
    }

    if (strcmp(tr[1], "erase") == 0) {
        printf("erase n1 n2 ...\n");
        printf("  Borra ficheros o directorios VACÍOS (usa unlink o rmdir).\n");
        printf("  Si el directorio no está vacío se informa del error.\n");
        return;
    }

    if (strcmp(tr[1], "delrec") == 0) {
        printf("delrec n1 n2 ...\n");
        printf("  Borra recursivamente ficheros y directorios (equivalente a rm -r).\n");
        return;
    }

    if (strcmp(tr[1], "open") == 0) {
        printf("open [archivo] modo\n");
        printf("  - Sin argumentos: lista archivos abiertos por el shell.\n");
        printf("  - open archivo modo : abre 'archivo' con el modo indicado y añade a la lista.\n");
        printf("  Modos admitidos: cr (O_CREAT|O_RDWR), ap (O_APPEND|O_WRONLY), ex (O_EXCL|O_CREAT|O_WRONLY),\n");
        printf("                 ro (O_RDONLY), rw (O_RDWR), wo (O_WRONLY), tr (O_TRUNC|O_WRONLY)\n");
        printf("  Ej: open fichero.txt cr\n");
        return;
    }

    if (strcmp(tr[1], "close") == 0) {
        printf("close df\n");
        printf("  Cierra el descriptor de fichero 'df' y lo elimina de la lista de abiertos.\n");
        return;
    }

    if (strcmp(tr[1], "dup") == 0) {
        printf("dup df\n");
        printf("  Duplica el descriptor 'df' (dup(2)) y añade el nuevo descriptor a la lista.\n");
        return;
    }

    if (strcmp(tr[1], "listopen") == 0) {
        printf("listopen\n");
        printf("  Muestra la lista de archivos abiertos por el shell (equivalente a 'open' sin args).\n");
        return;
    }

    if (strcmp(tr[1], "writestr") == 0) {
        printf("writestr df \"str\"\n");
        printf("  Escribe la cadena str en el descriptor df usando write(2).\n");
        printf("  Ej: writestr 3 \"Hola mundo\"\n");
        return;
    }

    if (strcmp(tr[1], "lseek") == 0) {
        printf("lseek df off ref\n");
        printf("  Sitúa el offset en el descriptor df. ref puede ser: SEEK_SET, SEEK_CUR, SEEK_END.\n");
        printf("  off puede ser un número (positivo/negativo si aplica). Imprime el nuevo offset.\n");
        printf("  Ej: lseek 3 100 SEEK_SET\n");
        return;
    }

    if (strcmp(tr[1], "infosys") == 0) {
        printf("infosys\n");
        printf("  Muestra información del sistema (sysname, nodename, release, version, machine) usando uname(2).\n");
        return;
    }

    if (strcmp(tr[1], "help") == 0) {
        printf("help [cmd]\n");
        printf("  Sin argumentos: lista los comandos disponibles.\n");
        printf("  Con 'cmd' muestra ayuda específica del comando.\n");
        return;
    }

    if (strcmp(tr[1], "quit") == 0 || strcmp(tr[1], "exit") == 0 || strcmp(tr[1], "bye") == 0) {
        printf("quit | exit | bye\n");
        printf("  Finaliza la ejecución del shell.\n");
        return;
    }

    
    printf("No hay ayuda específica para '%s'.\n", tr[1]);
}

void Cmd_date(char *tr[]){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char buffer[128];

    if (tr == NULL || tr[0] == NULL) return;

    if (tr[1] == NULL) {
        strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &tm);
        printf("%s\n", buffer);
    } else if (strcmp(tr[1], "-d") == 0) {
        strftime(buffer, sizeof(buffer), "%d/%m/%Y", &tm);
        printf("%s\n", buffer);
    } else if (strcmp(tr[1], "-t") == 0) {
        strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
        printf("%s\n", buffer);
    } else {
        printf("Uso: date [-d|-t]\n");
    }
}

void Cmd_hour(char *trozos[]) {
    
    if (trozos != NULL && trozos[1] != NULL) {
        fprintf(stderr, "Uso: hour\n"); 
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    printf("%s\n", buffer);
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


/* ==========================================================================
   SECCIÓN 2: GESTIÓN DE DIRECTORIOS Y SISTEMA DE FICHEROS
   ========================================================================== */

   // Navegación y creación
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

void Cmd_create(char *tr[]) {
    
    if (tr[1] == NULL) {
        fprintf(stderr, "Uso: create [-f] nombre\n");
        return;
    }

    
    if (strcmp(tr[1], "-f") == 0) {
        if (tr[2] == NULL) {
            fprintf(stderr, "Uso: create -f nombre\n");
            return;
        }

        FILE *file = fopen(tr[2], "w");
        if (file == NULL) {
            perror(tr[2]);
            return;
        }

        if (fclose(file) == EOF) {
            perror("fclose");
        }
        return; 
    }

    
    else if (tr[2] == NULL) {
        int result = mkdir(tr[1], 0755);
        if (result != 0) {
            perror(tr[1]);
        }
        return;
    }

    else {
        fprintf(stderr, "Uso: create [-f] nombre\n");
    }
}


// Configuración de listado (dir
void Cmd_setdirparams(char *tr[]) {
    if (tr == NULL || tr[0] == NULL) return;

    if (tr[1] == NULL) {
        fprintf(stderr, "Uso: setdirparams [long|short|link|nolink|hid|nohid|reca|recb|norec]\n");
        return;
    }

    if (strcmp(tr[1], "short") == 0) {
        dir_params.formato_largo = SHORT;
        printf("Parámetro de formato de listado establecido a 'short'\n");
    } 
    else if (strcmp(tr[1], "long") == 0) {
        dir_params.formato_largo = LONG;
        printf("Parámetro de formato de listado establecido a 'long'\n");
    } 
    else if (strcmp(tr[1], "link") == 0) {
        dir_params.mostrar_link = LINK;
        printf("Parámetro de enlaces simbólicos establecido a 'link'\n");
    } 
    else if (strcmp(tr[1], "nolink") == 0) {
        dir_params.mostrar_link = NOLINK;
        printf("Parámetro de enlaces simbólicos establecido a 'nolink'\n");
    } 
    else if (strcmp(tr[1], "hid") == 0) {
        dir_params.mostrar_ocultos = HID;
        printf("Parámetro de archivos ocultos establecido a 'hid'\n");
    } 
    else if (strcmp(tr[1], "nohid") == 0) {
        dir_params.mostrar_ocultos = NOHID;
        printf("Parámetro de archivos ocultos establecido a 'nohid'\n");
    } 
    else if (strcmp(tr[1], "reca") == 0) {
        dir_params.recursivo = RECA;
        printf("Parámetro de recursión establecido a 'reca'\n");
    } 
    else if (strcmp(tr[1], "recb") == 0) {
        dir_params.recursivo = RECB;
        printf("Parámetro de recursión establecido a 'recb'\n");
    } 
    else if (strcmp(tr[1], "norec") == 0) {
        dir_params.recursivo = NOREC;
        printf("Parámetro de recursión establecido a 'norec'\n");
    } 
    else {
        fprintf(stderr, "Parámetro desconocido: %s\n", tr[1]);
    }
}

void Cmd_getdirparams(char *trozos[]) {
    
    if (trozos != NULL && trozos[1] != NULL) {
        fprintf(stderr, "Uso: getdirparams\n");
        return;
    }

    const char *formatos[] = {"SHORT", "LONG"};
    const char *links[]    = {"NOLINK", "LINK"};
    const char *hids[]     = {"NOHID", "HID"};
    const char *recs[]     = {"NOREC", "RECB", "RECA"};

    printf("Listado: %s %s %s %s\n",
           formatos[dir_params.formato_largo],
           links[dir_params.mostrar_link],
           hids[dir_params.mostrar_ocultos],
           recs[dir_params.recursivo]);
}


// Listado y eliminación
void Cmd_dir(char *trozos[]) {
    if (trozos[1] == NULL) {
        ListarDirectorio(".");
        return;
    }
    for (int i = 1; trozos[i] != NULL; i++) {
        ListarDirectorio(trozos[i]);
    }
}

void Cmd_dir_d(char *trozos[]) {
    if (trozos[2] == NULL) {
        RecorrerDirectorioRecursivo(".");
        return;
    }
    for (int i = 2; trozos[i] != NULL; i++) {
        RecorrerDirectorioRecursivo(trozos[i]);
    }
}

void Cmd_erase(char *trozos[]) {
    if (trozos[1] == NULL) {
        fprintf(stderr, "Uso: erase n1 n2 ...\n");
        return;
    }
    for (int i = 1; trozos[i] != NULL; i++) {
        struct stat st;
        if (lstat(trozos[i], &st) == -1) {
            perror(trozos[i]);
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            if (rmdir(trozos[i]) == -1) {       //solo funciona si el directorio está vacío.
                perror(trozos[i]);
            }
        } else {
            if (unlink(trozos[i]) == -1) {      //**Si no es directorio** → lo trata como archivo
                perror(trozos[i]);
            }
        }
    }
}

void Cmd_delrec(char *trozos[]) {
    if (trozos[1] == NULL) {
        fprintf(stderr, "Uso: delrec n1 n2 ...\n");
        return;
    }
    for (int i = 1; trozos[i] != NULL; i++) {
        if (borrar_recursivo(trozos[i]) == -1) {
            fprintf(stderr, "Error borrando: %s\n", trozos[i]);
        }
    }
}

/* ==========================================================================
   SECCIÓN 3: GESTIÓN DE ARCHIVOS ABIERTOS
   ========================================================================== */

   // Operaciones sobre la tabla de archivos abiertos (OpenFiles)
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
    else if (strcmp(modo, "rw") == 0) flags = O_RDWR;                           //lectura y escritura.
    else if (strcmp(modo, "wo") == 0) flags = O_WRONLY;                         //sólo escritura.
    else if (strcmp(modo, "tr") == 0) flags = O_TRUNC | O_WRONLY;               //abrir sólo para escritura y truncar el archivo a cero.
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

// Operaciones de E/S básicas sobre descriptores
void Cmd_writestr(char *tr[]) {
    if (tr[1] == NULL || tr[2] == NULL) {
        fprintf(stderr, "Uso: writestr df str\n");
        return;
    }

    int df = atoi(tr[1]);                                       //descriptor
    ssize_t bytes_written = write(df, tr[2], strlen(tr[2]));    //write(df, tr[2] puntero a la cadena, cantidad de bytes pa escribir)
    if (bytes_written == -1) {
        perror("Error al escribir en el fichero");
    } else {
        printf("Se han escrito %zd bytes en el descriptor %d.\n", bytes_written, df); //muestra cuantos bytes se escribieron y la posicion
    }
}

void Cmd_lseek(char *tr[]) {
    if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
        fprintf(stderr, "Uso: lseek df off ref\n");
        return;
    }

    int df = atoi(tr[1]);               //descriptor del archivo
    off_t off = atol(tr[2]);            //desplazamiento
    int ref;                            //referencia para el desplazamiento (SET CUR END)

    //traduce la cadena de referencia del usuario a constantes del sistema
    if (strcmp(tr[3], "SEEK_SET") == 0)
        ref = SEEK_SET;                         //desde el inicio del archivo
    else if (strcmp(tr[3], "SEEK_CUR") == 0)
        ref = SEEK_CUR;                         //desde la posicion actual
    else if (strcmp(tr[3], "SEEK_END") == 0)
        ref = SEEK_END;                         //desde el final del archivo
    else {
        fprintf(stderr, "Referencias permitidas: SEEK_SET | SEEK_CUR | SEEK_END\n");
        return;
    }

    off_t pos = lseek(df, off, ref);                //devuelve la posicion absoluta
    if (pos == (off_t)-1) {
        perror("Error en lseek");
    } else {
        printf("Nuevo offset: %ld\n", (long)pos);   // devuelve pa pos de lectura escritura actual
    }
}

/* ==========================================================================
   SECCIÓN 4: GESTIÓN DE MEMORIA
   ========================================================================== */

// Comandos para reservar y liberar bloques
void Cmd_malloc(char *tr[], ListaMemoria *lm) {

    if (tr[1] == NULL) {
        printf("******Lista de bloques asignados malloc para el proceso %d\n", getpid());
        printListaMemoria(lm, MALLOC);
        return;
    }

    if (strcmp(tr[1], "-free") == 0) {
        if (tr[2] == NULL) {
            printf("******Lista de bloques asignados malloc para el proceso %d\n", getpid());
            printListaMemoria(lm, MALLOC);
            return;
        }

        size_t tam = (size_t)strtoul(tr[2], NULL, 10);
        if (tam == 0) {
            printf("No hay bloques de tamaño 0\n");
            return;
        }

        int encontrado = 0;
        for (int i = 0; i < lm->tamano; i++) {

            if (lm->data[i].tipo == MALLOC && lm->data[i].tamano == tam) {
                void *dir = lm->data[i].direccion;

                printf("Liberados %lu bytes en %p\n", tam, dir);
                
                removeBloqueMemoria(lm, dir);
                
                free(dir); 
                
                encontrado = 1;

                break;
            }
        }
        if (!encontrado) {
            printf("No hay bloque de ese tamaño asignado con malloc\n");
        }
        return;
    }
    size_t tam = (size_t)strtoul(tr[1], NULL, 10);
    if (tam == 0) {
        printf("No se asignan bloques de 0 bytes\n");
        return;
    }
    void *dir = malloc(tam);
    if (dir == NULL) {
        perror("Error al asignar memoria");
        return;
    }
    addBloqueMemoria(lm, dir, tam, MALLOC, 0, NULL, 0);
    printf("Asignados %lu bytes en %p\n", tam, dir);
}

void Cmd_shared(char *tr[], ListaMemoria *lm) {

    key_t cl;
    size_t tam;
    void *p;

    if (tr[1] == NULL) {
        printListaMemoria(lm, SHARED);
        return;
    }

    if (strcmp(tr[1], "-create") == 0) {
        if (tr[2] == NULL || tr[3] == NULL) {
            printListaMemoria(lm, SHARED);
            return;
        }
        cl = (key_t)strtoul(tr[2], NULL, 10);
        tam = (size_t)strtoul(tr[3], NULL, 10);
        if (tam == 0) {
            printf("No se asignan bloques de 0 bytes\n");
            return;
        }
        if ((p = ObtenerMemoriaShmget(cl, tam, lm)) != NULL)
            printf("Asignados %lu bytes en %p\n", (unsigned long)tam, p);
        else
            printf("Imposible asignar memoria compartida clave %lu:%s\n", (unsigned long)cl, strerror(errno));
    }
    else if (strcmp(tr[1], "-delkey") == 0) {
        if (tr[2] == NULL) {
            printf("Uso: shared -delkey cl\n");
            return;
        }
        cl = (key_t)strtoul(tr[2], NULL, 10);
        int id;
        if ((id = shmget(cl, 0, 0666)) == -1) {
            perror("shmget: imposible obtener memoria compartida");
            return;
        }
        if (shmctl(id, IPC_RMID, NULL) == -1)
            perror("shmctl: imposible eliminar memoria compartida\n");
        else
            printf("Clave %d eliminada de memoria compartida del sistema\n", cl);
    }
    else if (strcmp(tr[1], "-free") == 0) {
        if (tr[2] == NULL) {
            printf("Uso: shared -free cl\n");
            return;
        }
        cl = (key_t)strtoul(tr[2], NULL, 10);

        int i;
        for (i = 0; i < lm->tamano; i++) {
            if (lm->data[i].tipo == SHARED && lm->data[i].info.key == cl) {
                p = lm->data[i].direccion;
                if (shmdt(p) == -1) perror("shmdt");
                printf("Desasignada memoria compartida de clave %d en %p\n", cl, p);
                removeBloqueMemoria(lm, p);
                return;
            }
        }
        printf("Clave %d no encontrada en la lista del shell\n", cl);
    }
    else {

        cl = (key_t)strtoul(tr[1], NULL, 10);
        if ((p = ObtenerMemoriaShmget(cl, 0, lm)) != NULL)
            printf("Asignada memoria compartida de clave %lu en %p\n", (unsigned long)cl, p);
        else
            printf("Imposible asignar memoria compartida clave %lu:%s\n", (unsigned long)cl, strerror(errno));
    }
}

void Cmd_mmap(char *arg[], ListaMemoria *lm) {
    char *perm;
    void *p;
    int protection = 0;

    if (arg[1] == NULL) {
        printListaMemoria(lm, MMAP);
        return;
    }

    if (strcmp(arg[1], "-free") == 0) {
         if (arg[2] == NULL) { printf("Uso: mmap -free fichero\n"); return;}

         for (int i = 0; i < lm->tamano; i++) {
            if (lm->data[i].tipo == MMAP && strcmp(lm->data[i].info.mmap_data.nombre_fich, arg[2]) == 0) {
                p = lm->data[i].direccion;
                munmap(p, lm->data[i].tamano);
                close(lm->data[i].info.mmap_data.fd);
                removeBloqueMemoria(lm, p);
                printf("Desmapeado fichero %s en %p\n", arg[2], p);
                return;
            }
        }
        printf("Fichero no encontrado en la lista\n");
        return;
    }

    if (arg[2] != NULL) {
        perm = arg[2];
        if (strlen(perm) < 4) {
            if (strchr(perm, 'r') != NULL) protection |= PROT_READ;
            if (strchr(perm, 'w') != NULL) protection |= PROT_WRITE;
            if (strchr(perm, 'x') != NULL) protection |= PROT_EXEC;
        }
    }
    
    if ((p = MapearFichero(arg[1], protection, lm)) == NULL)
        perror("Imposible mapear fichero");
    else
        printf("fichero %s mapeado en %p\n", arg[1], p);
}

void Cmd_free(char *tr[], ListaMemoria *lm) {
    if (tr[1] == NULL) {
        printf("Uso: free direccion_memoria\n");
        return;
    }

    void *ptr = CadenaToPointer(tr[1]);


    BloqueMemoria *bloque = NULL;
    for (int i = 0; i < lm->tamano; i++) {
        if (lm->data[i].direccion == ptr) {
            bloque = &lm->data[i];
            break;
        }
    }

    if (bloque == NULL) {
        printf("La direccion %p no gestionada por el shell (no está en la lista)\n", ptr);
        return;
    }

    printf("Liberando bloque en %p de tipo ", ptr);


    if (bloque->tipo == MALLOC) {
        printf("malloc\n");
        free(ptr);
    } 
    else if (bloque->tipo == SHARED) {
        printf("shared\n");
        if (shmdt(ptr) == -1) perror("shmdt");
    } 
    else if (bloque->tipo == MMAP) {
        printf("mmap\n");
        if (munmap(ptr, bloque->tamano) == -1) perror("munmap");
        close(bloque->info.mmap_data.fd); 
    }


    removeBloqueMemoria(lm, ptr);
}

/* ==========================================================================
   SECCIÓN 5: MANIPULACIÓN DE DATOS EN MEMORIA
   ========================================================================== */

// Relleno y volcado de memoria
void Cmd_memfill(char *tr[], ListaMemoria *lm) {
    if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
        printf("Uso: memfill addr cont ch\n");
        return;
    }

    void *addr = CadenaToPointer(tr[1]);
    size_t cont = (size_t)strtoul(tr[2], NULL, 10);
    char ch = tr[3][0];

    if (cont == 0) {
        printf("Error: Contenido a rellenar debe ser > 0\n");
        return;
    }

    printf("Rellenando %lu bytes desde %p con caracter '%c'\n", 
           (unsigned long)cont, addr, ch);

    LlenarMemoria(addr, cont, (unsigned char)ch);
}

void Cmd_memdump(char *tr[], ListaMemoria *lm) {
    if (tr[1] == NULL || tr[2] == NULL) {
        printf("Uso: memdump addr cont\n");
        return;
    }

    void *addr = CadenaToPointer(tr[1]);
    size_t cont = (size_t)strtoul(tr[2], NULL, 10);
    unsigned char *p = (unsigned char *)addr;
    size_t i, j;
    
    if (cont == 0) return;

    printf("Volcando %lu bytes desde la direccion %p\n", (unsigned long)cont, addr);
    
    
    for (i = 0; i < cont; i += 20) {
        
        printf("%p->  ", p + i);
        for (j = 0; j < 20; j++) {
            if (i + j < cont) {
                unsigned char byte = p[i + j];

                if (byte == '\n') printf("\\n ");
                else if (byte == '\t') printf("\\t ");
                else if (byte == '\r') printf("\\r ");
                else if (isprint(byte)) printf(" %c ", byte);
                else printf("   ");
            }
        }
        printf("\n");
        printf("%p->  ", p + i);
        for (j = 0; j < 20; j++) {
            if (i + j < cont) {
                printf("%02x ", p[i + j]);
            }
        }
        printf("\n");
    }
}


// Transferencia de datos (Archivo <-> Memoria)
void Cmd_readfile(char *tr[], ListaMemoria *lm) {

    void *p;
    size_t cont = (size_t)-1;
    ssize_t n;

    if (tr[1] == NULL || tr[2] == NULL) {
        printf("Uso: readfile file addr cont\n");
        return;
    }

    p = CadenaToPointer(tr[2]);
    char *fichero = tr[1];

    if (tr[3] != NULL)
        cont = (size_t)strtoul(tr[3], NULL, 10);

    if ((n = LeerFichero(fichero, p, cont)) == -1)
        perror("Imposible leer fichero");
    else
        printf("Leidos %ld bytes de %s en %p\n", (long)n, fichero, p);
}

void Cmd_writefile(char *tr[], ListaMemoria *lm) {

    void *p;
    size_t cont;
    ssize_t n;

    if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
        printf("Uso: writefile file addr cont\n");
        return;
    }

    p = CadenaToPointer(tr[2]);
    cont = (size_t)strtoul(tr[3], NULL, 10);
    char *fichero = tr[1];

    if (cont == 0) {
        printf("Error: Contenido a escribir debe ser > 0\n");
        return;
    }

    if ((n = EscribirFichero(fichero, p, cont)) == -1)
        perror("Imposible escribir fichero");
    else
        printf("Escritos %ld bytes de %p en %s\n", (long)n, p, fichero);
}


// Lectura/Escritura usando descriptores de archivo (fd)
void Cmd_read(char *tr[], ListaMemoria *lm) {
    if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
        printf("Uso: read df addr cont\n");
        return;
    }

    int df = atoi(tr[1]);
    void *addr = CadenaToPointer(tr[2]);
    size_t cont = (size_t)strtoul(tr[3], NULL, 10);
    ssize_t n;

    if ((n = read(df, addr, cont)) == -1) {
        perror("Error al leer de descriptor");
    } else {
        printf("Leidos %ld bytes del descriptor %d en %p\n", (long)n, df, addr);
    }
}

void Cmd_write(char *tr[], ListaMemoria *lm) {

    if (tr[1] == NULL || tr[2] == NULL || tr[3] == NULL) {
        printf("Uso: write df addr cont\n");
        return;
    }

    int df = atoi(tr[1]);
    void *addr = CadenaToPointer(tr[2]);
    size_t cont = (size_t)strtoul(tr[3], NULL, 10);
    ssize_t n;

    if ((n = write(df, addr, cont)) == -1) {
        perror("Error al escribir en descriptor");
    } else {
        printf("Escritos %ld bytes de %p en el descriptor %d\n", (long)n, addr, df);
    }
}

/* ==========================================================================
   SECCIÓN 6: INFORMACIÓN DEL PROCESO Y MEMORIA
   ========================================================================== */

void Cmd_recurse(char *tr[], ListaMemoria *lm) {
    if (tr[1] == NULL) return;
    int n = atoi(tr[1]);
    if (n > 0)
        Recursiva(n);
}

void Cmd_mem(char *tr[], ListaMemoria *lm) {

    int local1 = 1, local2 = 2, local3 = 3;
    static int st1 = 10, st2 = 20, st3 = 30;
    static int st_u1, st_u2, st_u3;

    int blocks = 0, funcs = 0, vars = 0, pmap = 0;


    if (tr[1] == NULL || strcmp(tr[1], "-all") == 0) {
        blocks = funcs = vars = 1; 
    } else if (strcmp(tr[1], "-blocks") == 0) blocks = 1;
    else if (strcmp(tr[1], "-funcs") == 0) funcs = 1;
    else if (strcmp(tr[1], "-vars") == 0) vars = 1;
    else if (strcmp(tr[1], "-pmap") == 0) pmap = 1;
    else {
        printf("Uso: mem [-blocks|-funcs|-vars|-all|-pmap]\n");
        return;
    }

    if (vars) {
        printf("Variables locales:\t%p, %p, %p\n", &local1, &local2, &local3);
        printf("Variables globales:\t%p, %p, %p\n", &v_global1, &v_global2, &v_global3);
        printf("Var (N.I.) globales:\t%p, %p, %p\n", &v_global_u1, &v_global_u2, &v_global_u3);
        printf("Variables estáticas:\t%p, %p, %p\n", &st1, &st2, &st3);
        printf("Var (N.I.) estáticas:\t%p, %p, %p\n", &st_u1, &st_u2, &st_u3);
    }

    if (funcs) {
        printf("Funciones programa:\t%p, %p, %p\n", (void *)Cmd_mem, (void *)Recursiva, (void *)TrocearCadena);
        printf("Funciones librería:\t%p, %p, %p\n", (void *)printf, (void *)malloc, (void *)strcmp);
    }

    if (blocks) {
        printf("******Lista de bloques asignados para el proceso %d\n", getpid());
        printListaMemoria(lm, -1);
    }

    if (pmap) {
        Do_pmap();
    }
}

/* ==========================================================================
   SECCIÓN 7: GESTIÓN DE CREDENCIALES DE USUARIO
   ========================================================================== */


void Cmd_uid(char *tr[]) {
    if (tr[1] == NULL || strcmp(tr[1], "-get") == 0) {
        MostrarCredenciales();
        return;
    }

    if (strcmp(tr[1], "-set") == 0) {
        if (tr[2] == NULL) {
            printf("Uso: uid -set [-l] id\n");
            return;
        }

        uid_t nuevo_uid;
        int es_login = 0;
        char *id_str;

        if (strcmp(tr[2], "-l") == 0) {
            if (tr[3] == NULL) {
                printf("Uso: uid -set -l loginname\n");
                return;
            }
            es_login = 1;
            id_str = tr[3];
        } else {
            id_str = tr[2];
        }

        if (es_login) {
            struct passwd *pw = getpwnam(id_str);
            if (pw == NULL) {
                printf("Usuario no encontrado: %s\n", id_str);
                return;
            }
            nuevo_uid = pw->pw_uid;
        } else {
            nuevo_uid = (uid_t)atoi(id_str);
        }

        if (seteuid(nuevo_uid) == -1) {
            perror("Imposible cambiar credencial");
        } else {
            printf("Credencial efectiva cambiada a %d\n", nuevo_uid);
        }
        return;
    }

    printf("Uso: uid [-get] | [-set [-l] id]\n");
}

void Cmd_showenv(char *tr[], char *envp[]) {
    if (tr[1] == NULL) {
        for (int i = 0; envp[i] != NULL; i++)
            printf("%p->main arg3[%d]=(%p) %s\n", &envp[i], i, envp[i], envp[i]);
    } 
    else if (strcmp(tr[1], "-environ") == 0) {

        for (int i = 0; environ[i] != NULL; i++)
             printf("%p->environ[%d]=(%p) %s\n", &environ[i], i, environ[i], environ[i]);
    }
    else if (strcmp(tr[1], "-addr") == 0) {

        printf("environ:   %p (almacenado en %p)\n", environ, &environ);
        printf("main arg3: %p (almacenado en %p)\n", envp, &envp);
    }
    else {
        printf("Uso: showenv [-environ|-addr]\n");
    }
}

void Cmd_envvar(char *tr[], char *envp[]) {
    if (tr[1] == NULL) {
        printf("Uso: envvar [-show | -change] ...\n");
        return;
    }

    if (strcmp(tr[1], "-show") == 0) {
        if (tr[2] == NULL) {
            printf("Falta el nombre de la variable\n");
            return;
        }
        
        char *nombre = tr[2];
        int pos;


        pos = BuscarVariable(nombre, envp);
        if (pos != -1)
            printf("Con arg3 main: %s (%p) @%p\n", envp[pos], envp[pos], &envp[pos]);
        else
            printf("Con arg3 main: NO EXISTE\n");


        pos = BuscarVariable(nombre, environ);
        if (pos != -1)
            printf("Con environ:   %s (%p) @%p\n", environ[pos], environ[pos], &environ[pos]);
        else
            printf("Con environ:   NO EXISTE\n");


        char *valor = getenv(nombre);
        if (valor)
            printf("Con getenv:    %s (%p)\n", valor, valor);
        else
            printf("Con getenv:    NO EXISTE\n");
    } 
    else if (strcmp(tr[1], "-change") == 0) {
        if (tr[2] == NULL || tr[3] == NULL || tr[4] == NULL) {
            printf("Uso: envvar -change [-a|-e|-p] var valor\n");
            return;
        }

        char *opcion = tr[2];
        char *nombre = tr[3];
        char *valor = tr[4];

        if (strcmp(opcion, "-a") == 0) {

            if (CambiarVariable(nombre, valor, envp) == -1)
                perror("Imposible cambiar variable en arg3");
        }
        else if (strcmp(opcion, "-e") == 0) {

            if (CambiarVariable(nombre, valor, environ) == -1)
                perror("Imposible cambiar variable en environ");
        }
        else if (strcmp(opcion, "-p") == 0) {

            char *nueva = malloc(strlen(nombre) + strlen(valor) + 2);
            if (nueva == NULL) {
                perror("malloc");
                return;
            }
            sprintf(nueva, "%s=%s", nombre, valor);
            if (putenv(nueva) != 0)
                perror("putenv");
        } 
        else {
            printf("Opcion invalida: %s\n", opcion);
        }
    }
    else {
        printf("Uso: envvar [-show | -change] ...\n");
    }
}

void Cmd_fork(char *tr[]) {
    pid_t pid;

    if ((pid = fork()) == 0) {

        printf("ejecutando proceso %d\n", getpid());
        exit(0);
    }
    else if (pid != -1) {

        waitpid(pid, NULL, 0); 
    }
    else {
        perror("fork");
    }
}

void Cmd_jobs(char *tr[], ListaProcesos *l) {
    printListaProcesos(l);
}

void Cmd_deljobs(char *tr[], ListaProcesos *l) {
    if (tr[1] == NULL) {
        printf("Uso: deljobs -term | -sig\n");
        return;
    }

    updateListaProcesos(l);

    for (int i = 0; i < l->tamano; i++) {
        int eliminar = 0;
        
        if (l->data[i].estado == TERMINADO && strcmp(tr[1], "-term") == 0) {
            eliminar = 1;
        }
        else if (l->data[i].estado == SENALADO && strcmp(tr[1], "-sig") == 0) {
            eliminar = 1;
        }

        if (eliminar) {

            removeProceso(l, l->data[i].pid);
            i--; 
        }
    }
}

void Cmd_exec(char *tr[]) {
    if (tr[1] == NULL) return;

    InfoEjecucion info = AnalizarProgSpec(&tr[1]);

    if (info.priority != -9999) {
        if (setpriority(PRIO_PROCESS, 0, info.priority) == -1) {
            perror("setpriority");
        }
    }

    execvp(info.argv_exec[0], info.argv_exec);
    perror("Imposible ejecutar");
}

void Cmd_lanzar(char *tr[], ListaProcesos *l) {

    InfoEjecucion info = AnalizarProgSpec(tr);
    pid_t pid;

    if ((pid = fork()) == 0) {
        
        if (info.priority != -9999) {

            if (setpriority(PRIO_PROCESS, 0, info.priority) == -1) {
                perror("setpriority");
            }
        }

        execvp(info.argv_exec[0], info.argv_exec);
        
        perror("Imposible ejecutar");
        exit(EXIT_FAILURE);
    } 
    else if (pid > 0) {
        
        if (info.background) {

            char *cmd_completo = ReconstruirComando(info.argv_exec);
            
            addProceso(l, pid, cmd_completo);
            printf("Proceso %d ejecutándose en segundo plano\n", pid);
        } else {
            waitpid(pid, NULL, 0);
        }
    } 
    else {
        perror("fork");
    }
}