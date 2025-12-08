# Compilador y flags
CC = gcc
CFLAGS = -Wall -g

# Archivos objeto
OBJ = p3.o comandos.o historial.o openfiles.o memoria.o

# Ejecutable
TARGET = p3

# Regla principal
all: $(TARGET)

# Cómo generar el ejecutable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Cómo compilar los .c a .o
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Limpiar archivos generados
clean:
	rm -f $(OBJ) $(TARGET)

# Regla de phony para que make no confunda nombres de archivos con objetivos
.PHONY: all clean