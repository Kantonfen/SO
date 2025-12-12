
#define _POSIX_C_SOURCE 200809L

#include "processlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>


static const char *NombreSenal(int sen) {
    switch (sen) {
        case SIGHUP: return "HUP";
        case SIGINT: return "INT";
        case SIGQUIT: return "QUIT";
        case SIGILL: return "ILL";
        case SIGTRAP: return "TRAP";
        case SIGABRT: return "ABRT";
        case SIGBUS: return "BUS";
        case SIGFPE: return "FPE";
        case SIGKILL: return "KILL";
        case SIGUSR1: return "USR1";
        case SIGSEGV: return "SEGV";
        case SIGUSR2: return "USR2";
        case SIGPIPE: return "PIPE";
        case SIGALRM: return "ALRM";
        case SIGTERM: return "TERM";
        case SIGCHLD: return "CHLD";
        case SIGCONT: return "CONT";
        case SIGSTOP: return "STOP";
        case SIGTSTP: return "TSTP";
        case SIGTTIN: return "TTIN";
        case SIGTTOU: return "TTOU";

        default: return "UNKNOWN";
    }
}

static int ObtenerPrioridad(pid_t pid) {
    errno = 0;
    int prio = getpriority(PRIO_PROCESS, pid);
    if (errno != 0) return -1000;
    return prio;
}

void initListaProcesos(ListaProcesos *l) {
    if (!l) return;
    l->capacidad = 10;
    l->tamano = 0;
    l->data = malloc(l->capacidad * sizeof(Proceso));
    if (!l->data) {
        perror("malloc lista procesos");
        exit(EXIT_FAILURE);
    }
}

void freeListaProcesos(ListaProcesos *l) {
    if (!l) return;
    for (int i = 0; i < l->tamano; i++) {
        free(l->data[i].comando);
    }
    free(l->data);
    l->data = NULL;
    l->tamano = 0;
    l->capacidad = 0;
}

void addProceso(ListaProcesos *l, pid_t pid, const char *cmd) {
    if (!l || !cmd) return;

    if (l->tamano == l->capacidad) {
        int nueva = l->capacidad * 2;
        Proceso *tmp = realloc(l->data, nueva * sizeof(Proceso));
        if (!tmp) {
            perror("realloc lista procesos");
            return;
        }
        l->data = tmp;
        l->capacidad = nueva;
    }


    l->data[l->tamano].pid = pid;
    l->data[l->tamano].comando = strdup(cmd);
    l->data[l->tamano].estado = ACTIVO; 
    l->data[l->tamano].info = 0;


    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(l->data[l->tamano].tiempo, sizeof(l->data[l->tamano].tiempo), 
             "%Y/%m/%d %H:%M:%S", tm_info);

    l->tamano++;
}

void removeProceso(ListaProcesos *l, pid_t pid) {
    if (!l) return;
    for (int i = 0; i < l->tamano; i++) {
        if (l->data[i].pid == pid) {
            free(l->data[i].comando);
            
            for (int j = i; j < l->tamano - 1; j++) {
                l->data[j] = l->data[j + 1];
            }
            l->tamano--;
            return; 
        }
    }
}

void updateListaProcesos(ListaProcesos *l) {
    if (!l) return;
    int status;
    pid_t pid_wait;

    for (int i = 0; i < l->tamano; i++) {

        if (l->data[i].estado == ACTIVO || l->data[i].estado == DETENIDO) {
            
            pid_wait = waitpid(l->data[i].pid, &status, WNOHANG | WUNTRACED | WCONTINUED);

            if (pid_wait == l->data[i].pid) {
                if (WIFEXITED(status)) {
                    l->data[i].estado = TERMINADO;
                    l->data[i].info = WEXITSTATUS(status);
                } 
                else if (WIFSIGNALED(status)) {
                    l->data[i].estado = SENALADO;
                    l->data[i].info = WTERMSIG(status);
                } 
                else if (WIFSTOPPED(status)) {
                    l->data[i].estado = DETENIDO;
                    l->data[i].info = WSTOPSIG(status);
                } 
                else if (WIFCONTINUED(status)) {
                    l->data[i].estado = ACTIVO;
                    l->data[i].info = 0;
                }
            }
        }
    }
}

void printListaProcesos(ListaProcesos *l) {
    if (!l) return;

    updateListaProcesos(l);

    for (int i = 0; i < l->tamano; i++) {
        Proceso *p = &l->data[i];
        int prio = ObtenerPrioridad(p->pid);

        printf("%d\t", p->pid);

        if (p->estado == ACTIVO || p->estado == DETENIDO) {
             printf("p=%d\t", prio);
        } else {
             printf("p=--\t");
        }

        printf("%s\t", p->tiempo);

        switch (p->estado) {
            case ACTIVO:
                printf("ACTIVE (%03d) ", p->info);
                break;
            case TERMINADO:
                printf("FINISHED (%03d) ", p->info);
                break;
            case DETENIDO:
                printf("STOPPED (%s) ", NombreSenal(p->info));
                break;
            case SENALADO:
                printf("SIGNALED (%s) ", NombreSenal(p->info));
                break;
        }

        printf("%s\n", p->comando);
    }
}