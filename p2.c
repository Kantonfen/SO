// Xoel Queiro Lema (login: xoel.queiro)
// Daniel Cabrera Herrera (login: daniel.cabrera)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "historial.h"
#include "comandos.h"
#include "openfiles.h"
#include "memoria.h"

#define MAX_INPUT_SIZE 1024

int main(void) {
    char input[MAX_INPUT_SIZE];
    int terminado = 0;


    Historial historial;
    initHistorial(&historial);


    OpenFiles archivosAbiertos;
    initOpenFiles(&archivosAbiertos);


    ListaMemoria memoria;
    initListaMemoria(&memoria);


    while (!terminado) {
        printf("shell > ");
        fflush(stdout);

        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            printf("\nSaliendo del Shell (fin de entrada)\n");
            break;
        }


        terminado = ProcesarEntrada(input, &historial, &archivosAbiertos, &memoria);
    }


    freeHistorial(&historial);
    freeOpenFiles(&archivosAbiertos);
    freeListaMemoria(&memoria);

    return 0;
}