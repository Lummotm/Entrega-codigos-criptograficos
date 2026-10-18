#!/usr/bin/env bash
set -euo pipefail

TARGET="main"
SRC="cifrador.c"
AUTHOR_FILE="utils/author.txt"
ARCHIVO_ENTREGA="cifrador__68.c" # nombre dentro del zip
ZIP_NAME="entrega_cripto.zip"
CC="gcc"
CFLAGS="-Wall -Wextra -O2"
TMP_DIR="tmp_entrega"

# Compilar
compile() {
  "$CC" $CFLAGS "$SRC" -o "$TARGET"
}

# Limpiar
clean() {
  rm -f "$TARGET" "$ZIP_NAME" *~ *.c_entrega
  rm -rf "$TMP_DIR"
}

# Empaquetar con autor inyectado
entrega() {
  if [[ ! -f "$AUTHOR_FILE" ]]; then
    echo "Error: No existe $AUTHOR_FILE. Créalo con tu nombre y apellidos." >&2
    exit 1
  fi

  clean
  mkdir -p "$TMP_DIR"

  echo "Inyectando autor desde $AUTHOR_FILE..."
  local author_name
  author_name="$(cat "$AUTHOR_FILE")"
  {
    echo "/* Autor: $author_name */"
    cat "$SRC"
  } >"$TMP_DIR/$ARCHIVO_ENTREGA"

  # Copiamos también lo que queramos que vaya en el zip
  # cp "build.sh" "$TMP_DIR/build.sh" 2>/dev/null || true
  # Si quieres incluir el test, el Makefile, etc., añádelo aquí:
  cp "test" "$TMP_DIR/test"

  echo "Creando $ZIP_NAME..."
  (
    cd "$TMP_DIR"
    zip -r "../$ZIP_NAME" .
  )

  rm -rf "$TMP_DIR"
  echo "¡Empaquetado completado con éxito!"
}

case "${1:-all}" in
all) compile ;;
compile) compile ;;
clean) clean ;;
entrega) entrega ;;
test) compile && ./test ;;
*)
  echo "Uso: $0 {all|compile|clean|entrega|test}" >&2
  exit 1
  ;;
esac
