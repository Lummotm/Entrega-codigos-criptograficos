# Nombre del binario y del archivo fuente base
TARGET = cifrador
SRC = entrega.c
AUTHOR_FILE = utils/author.txt
ZIP_NAME = entrega_cripto.zip

# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Regla para empaquetar inyectando el autor discretamente
entrega: clean
	@if [ ! -f $(AUTHOR_FILE) ]; then \
		echo "Error: No existe el archivo $(AUTHOR_FILE). Créalo con tu nombre y apellidos."; \
		exit 1; \
	fi
	@echo "Inyectando autor desde $(AUTHOR_FILE)..."
	@AUTHOR_NAME=$$(cat $(AUTHOR_FILE)); \
	echo "/* Autor: $$AUTHOR_NAME */" > tmp_file.c; \
	cat $(SRC) >> tmp_file.c; \
	mv tmp_file.c $(SRC)_entrega.c
	
	@echo "Creando el archivo $(ZIP_NAME)..."
	zip -r $(ZIP_NAME) $(SRC)_entrega.c Makefile 
	@rm -f $(SRC)_entrega.c
	@echo "¡Empaquetado completado con éxito!"

clean:
	rm -f $(TARGET) $(ZIP_NAME) *~ tmp_file.c *_entrega.c

.PHONY: all zip clean
