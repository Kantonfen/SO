// Xoel Queiro Lema
// Daniel Cabrera Herrera

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

    // 1. Inicializar Historial
    Historial historial;
    initHistorial(&historial);

    // 2. Inicializar OpenFiles
    OpenFiles archivosAbiertos;
    initOpenFiles(&archivosAbiertos);

    // 3. Inicializar Memoria (Nueva estructura)
    ListaMemoria memoria;
    initListaMemoria(&memoria);

    // 4. Bucle del shell
    while (!terminado) {
        printf("shell > ");
        fflush(stdout);

        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            printf("\nSaliendo del Shell (fin de entrada)\n");
            break;
        }

        // Llamamos a nuestra nueva función "cerebro"
        // Si devuelve 1, terminado se vuelve 1 y el bucle acaba.
        terminado = ProcesarEntrada(input, &historial, &archivosAbiertos, &memoria);
    }

    // 5. Liberar memoria al salir
    freeHistorial(&historial);
    freeOpenFiles(&archivosAbiertos);
    freeListaMemoria(&memoria);

    return 0;
}