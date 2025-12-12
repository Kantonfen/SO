// Xoel Queiro Lema (login: xoel.queiro)
// Daniel Cabrera Herrera (login: daniel.cabrera)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "historial.h"
#include "comandos.h"
#include "openfiles.h"
#include "memoria.h"
#include "processlist.h"

#define MAX_INPUT_SIZE 1024

int main(int argc, char *argv[], char *envp[]) {
    char input[MAX_INPUT_SIZE];
    int terminado = 0;


    Historial historial;
    initHistorial(&historial);


    OpenFiles archivosAbiertos;
    initOpenFiles(&archivosAbiertos);


    ListaMemoria memoria;
    initListaMemoria(&memoria);

    ListaProcesos procesos;
    initListaProcesos(&procesos);

    while (!terminado) {
        printf("shell > ");
        fflush(stdout);

        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            printf("\nSaliendo del Shell (fin de entrada)\n");
            break;
        }


        terminado = ProcesarEntrada(input, &historial, &archivosAbiertos, &memoria, envp);
    }


    freeHistorial(&historial);
    freeOpenFiles(&archivosAbiertos);
    freeListaMemoria(&memoria);
    freeListaProcesos(&procesos);

    return 0;
}