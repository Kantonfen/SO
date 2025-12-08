
# Sistemas Operativos
## Grado en Informática 2025/2026
### Práctica de Laboratorio 2: Memoria

Continuamos codificando el shell que comenzamos en la primera práctica de laboratorio. Ahora el shell tendrá la capacidad de asignar o desasignar bloques de memoria vía `malloc`, mapeando ficheros o memoria compartida. El shell mantendrá una lista de los bloques de memoria asignados (solo aquellos asignados con los comandos `malloc`, `shared` o `mmap`, no los que necesite asignar para su funcionamiento normal).

### Tabla de Comandos

| Comando | Param | Descripción |
| :--- | :--- | :--- |
| **malloc** | `n` | Asigna un bloque de memoria malloc de tamaño `n` bytes. Actualiza la lista de bloques de memoria. |
| **malloc** | `-free n` | Desasigna un bloque de memoria malloc de tamaño `n` (siempre que haya sido previamente asignado con `allocate` [sic]). Actualiza la lista de bloques de memoria. Si hay más de un bloque de tamaño `n` asignado, desasigna UNO de ellos. |
| **malloc** | | Lista los bloques asignados con el comando `malloc`. |
| **mmap** | `fich perm` | Mapea el fichero `fich` en memoria con permisos `perm`. (`perm` está en el formato `rwx`, `---`). Actualiza la lista de bloques de memoria. |
| **mmap** | `-free fich` | Desmapea el fichero `fich` de la memoria (siempre que haya sido previamente mapeado). Actualiza la lista de bloques de memoria. Si `fich` ha sido mapeado varias veces, desmapea UNO de ellos. |
| **mmap** | | Lista los bloques asignados con el comando `mmap`. |
| **shared** | `-create cl n` | Crea un bloque de memoria compartida con clave `cl` y tamaño `n` y lo adjunta al espacio de direcciones del proceso. Actualiza la lista de bloques de memoria. |
| **shared** | `cl` | Adjunta un bloque de memoria compartida con clave `cl` al espacio de direcciones del proceso (el bloque debe estar ya creado pero no necesariamente adjunto al espacio del proceso). Actualiza la lista de bloques de memoria. |
| **shared** | `-free cl` | Desadjunta un bloque de memoria compartida de clave `cl` (siempre que haya sido previamente asignado). Actualiza la lista de bloques de memoria. |
| **shared** | `-delkey cl` | Elimina el bloque de memoria de clave `cl` del sistema. NO DESADJUNTA LA MEMORIA COMPARTIDA CON ESA CLAVE SI ESTUVIERA ADJUNTA. |
| **shared** | | Lista los bloques asignados con el comando `shared`. |
| **free** | `addr` | Desasigna el bloque con dirección `addr`. (si es un bloque malloc, lo libera; si es un bloque de memoria compartida, lo desadjunta...). Actualiza la lista de bloques de memoria. |
| **memfill** | `addr cont ch` | Llena la memoria con el carácter `ch`, comenzando en la dirección `addr`, por `cont` bytes. |
| **memdump** | `addr cont` | Vuelca el contenido de `cont` bytes de memoria en la dirección `addr` a la pantalla. Vuelca códigos hex, y para caracteres imprimibles el carácter asociado. Salto de línea, tabuladores, retornos de carro, deben imprimirse como en C: `\n`, `\t` y `\r`. Los caracteres no imprimibles deben imprimirse como espacios en blanco (para mantener la correspondencia con los códigos hex). |
| **mem** | `-funcs` | Imprime las direcciones de 3 funciones del programa y 3 funciones de librería. |
| **mem** | `-vars` | Imprime las direcciones de 3 variables externas, 3 externas inicializadas, 3 estáticas, 3 estáticas inicializadas y 3 automáticas. |
| **mem** | `-blocks` | Imprime la lista de bloques asignados (con `malloc`, `shared` y `pmap`). |
| **mem** | `-all` | Imprime todo lo anterior (`-funcs`, `-vars` y `-blocks`). |
| **mem** | `-pmap` | Muestra la salida del comando `pmap` para el proceso shell (`vmmap` en macos). |
| **readfile** | `file addr cont` | Lee `cont` bytes de un fichero hacia la dirección de memoria `addr`. |
| **writefile** | `file addr cont` | Escribe en un fichero `cont` bytes comenzando en la dirección de memoria `addr`. |
| **read** | `df addr cont` | Lo mismo que `readfile` pero usamos un descriptor de fichero (ya abierto). |
| **write** | `df addr cont` | Lo mismo que `writefile` pero usamos un descriptor de fichero (ya abierto). |
| **recurse** | `n` | Ejecuta la función recursiva `n` veces. Esta función tiene un array automático de tamaño 1024, un array estático de tamaño 1024 e imprime las direcciones de ambos arrays y su parámetro (así como el número de recursión) antes de llamarse a sí misma. |

---

### IMPORTANTE:

Tenemos que implementar (implementación de lista libre) una lista de bloques de memoria. Para cada bloque debemos almacenar:
* Su dirección de memoria
* Su tamaño
* Hora de asignación
* Tipo de asignación (memoria malloc, memoria compartida, fichero mapeado)
* Otra info: clave para bloques de memoria compartida, nombre del fichero y descriptor de fichero para ficheros mapeados.

Los comandos del shell `malloc`, `shared` y `mmap` asignan y desasignan bloques de memoria y los añaden (o eliminan) de la lista.
Cada elemento en la lista tiene info de un bloque de memoria que creamos con los comandos del shell `malloc`, `shared` y `mmap`. La información sobre ese bloque es la previamente establecida.

Trataremos con tres tipos de bloques de memoria:

1.  **memoria malloc:** esta es la memoria más común que usamos, la asignamos con la función de librería `malloc` y la desasignamos con la función de librería `free`.
2.  **memoria compartida:** esta es memoria que puede ser compartida entre varios procesos. La memoria es identificada por un número (llamado clave) de modo que dos procesos usando la misma clave obtienen el mismo bloque de memoria. Usamos la llamada al sistema `shmget` para obtener la memoria y `shmat` y `shmdt` para colocarla en (o desadjuntarla de) el espacio de direcciones del proceso. `shmget` necesita la clave, el tamaño y los flags. Usaremos `flags=IPC_CREAT | IPC_EXCL | permisos` para crear una nueva (da error si ya existe) y `flags=permisos` para usar una ya creada. Para borrar una clave usaremos el comando `shared -delkey` (este comando no desasigna nada, solo borra la clave). El estado de la memoria compartida en el sistema puede comprobarse vía línea de comandos con el comando `ipcs`. Se proporciona un archivo C adicional (`ayudaP2-26.txt`) con algunas funciones útiles.
3.  **ficheros mapeados:** También podemos mapear ficheros en memoria de modo que aparezcan en el espacio de direcciones de un proceso. Las llamadas al sistema `mmap` y `munmap` hacen el truco. De nuevo, se proporciona el archivo C adicional (`ayudaP2.txt`) con algunas funciones útiles.

El contenido de nuestra lista debe ser compatible con lo que el sistema muestra con el comando `pmap` (`procstat vm`, `vmmap`... dependiendo de la plataforma).

La función recursiva tiene un array estático y uno dinámico del mismo tamaño (1024 bytes) e imprime sus direcciones junto con la dirección y valor del parámetro.

### ERRORES DE TIEMPO DE EJECUCIÓN LEGÍTIMOS

Aunque NO SE PERMITIRÁN ERRORES DE TIEMPO DE EJECUCIÓN (segmentation, bus error...) y los programas con errores de tiempo de ejecución no obtendrán puntuación, este programa puede producir legítimamente errores de fallo de segmentación en escenarios tales como estos:
* `memdump` o `memfill` intentan acceder a una dirección inválida suministrada a través de la línea de comandos.
* `memfill`, `readfile` o `read` corrompen la pila de usuario o el heap.

### RECUERDA:

* Información sobre las llamadas al sistema y funciones de librería necesarias para codificar estos programas está disponible a través de man: (`shmget`, `shmat`, `malloc`, `free`, `mmap`, `munmap`, `shmctl`, `open`, `read`, `write`, `close`, `sscanf`...).
* Se proporciona un shell de referencia (para varias plataformas) para que los estudiantes comprueben cómo debería comportarse el shell. Este programa debe ser verificado para averiguar la sintaxis y comportamiento de los varios comandos. POR FAVOR DESCARGA LA ÚLTIMA VERSIÓN (el comando `version`, en el shell de referencia más nuevo, muestra qué versión estás usando).
* El programa debe compilar limpiamente (no producir advertencias incluso cuando se compila con `gcc -Wall`).
* Estos programas no pueden tener fugas de memoria (puedes usar `valgrind` para comprobar).
* Cuando el programa no pueda realizar su tarea (por cualquier razón, por ejemplo, falta de privilegios) debe informar al usuario (Ver sección de errores).
* Toda la entrada y salida se realiza a través de la entrada y salida estándar.
* Los errores deben ser tratados como en la práctica de laboratorio anterior.

### ENTREGA DEL TRABAJO

* El trabajo debe hacerse en parejas.
* Se usará Moodle para entregar el código fuente: un archivo zip conteniendo un directorio llamado `P2` donde residen todos los archivos fuente de la práctica de laboratorio.
* El nombre del programa principal será `p2.c`. El programa debe poder compilarse con `gcc p2.c`. Opcionalmente se puede suministrar un `Makefile` para que todo el código fuente pueda compilarse con solo `make`.
* Si ese fuera el caso, el programa compilado debe llamarse `p2` y colocarse en el mismo directorio que las fuentes (sin directorios de construcción o similares).
* SOLO UNO DE LOS MIEMBROS DEL GRUPO entregará el código fuente.
* Los nombres y logins de todos los miembros del grupo deben aparecer en el código fuente de los programas principales (en la parte superior del archivo).
* Los trabajos entregados que no cumplan con estas reglas serán descartados.

**FECHA LÍMITE: 23:00, viernes 21 de noviembre de 2025**

### COSAS INTERESANTES PARA HACER CON ESTA PRÁCTICA DE LABORATORIO

* Asignar 3Gb de memoria con `malloc`. Comprobar dónde está en el espacio de direcciones.
* Asignar memoria compartida, leer el archivo de código del shell en esa dirección. En otra terminal ejecutar otra copia del shell, asignar memoria compartida con la misma clave. Volcar el contenido de esa memoria a la pantalla.
* Comprobar cuántas iteraciones de la función recursiva son necesarias para desbordar la pila. Cambiar el tamaño del array automático y/o estático a 4096 (en lugar de los 1024). Cuántas iteraciones son necesarias ahora para desbordar la pila.
* Mapear un fichero de texto a memoria, comprobar el espacio de direcciones del proceso, volcar la dirección donde el fichero está mapeado. Si obtienes "segmentation fault".... ¿has comprobado los permisos de mapeo?
* Intentar volcar o llenar (con `memdump` o `memfill`) direcciones que no están en el espacio del shell.
* Compilar el shell con el flag `-static`, comprobar la diferencia de espacios de direcciones. Comprobar dónde en esos espacios de direcciones están las diferentes variables y funciones.
* (En linux) Asignar con `malloc` un bloque mayor de 256k, (por ejemplo 500000000), linux devolverá una dirección tal como `0xNNNNNNNNNNNNN010`, y el comando `pmap` mostraría `NNNNNNNNNNNNN000`. Llena la memoria en `0xNNNNNNNNNNNNN000` con cualquier carácter (por al menos 16 bytes). Ahora libera el bloque (`malloc -free`) en `0xNNNNNNNNNNNNN010`.