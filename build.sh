#!/usr/bin/env bash
set -uo pipefail

TARGET="main"
SRC="cifrador.c"
AUTHOR_FILE="utils/author.txt"
ARCHIVO_ENTREGA="cifrador__68.c" # nombre dentro del zip
TEST_SRC="cifrador-tests.c"
TEST_BIN="main_tests"
TEST_FILE="utils/test.sh"
ZIP_NAME="entrega_cripto.zip"
CC="gcc"
CFLAGS="-Wall -Wextra -O2"
TMP_DIR="tmp_entrega"

# Compilar
compile() { "$CC" $CFLAGS "$SRC" -o "$TARGET"; }

compile_tests() { "$CC" $CFLAGS "$TEST_SRC" -o "$TEST_BIN"; }

# Limpiar
clean() {
  rm -f "$TARGET" "$ZIP_NAME" *~ *.c_entrega
  rm main_*
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

  echo "Creando $ZIP_NAME..."
  (
    cd "$TMP_DIR" || exit
    zip -r "../$ZIP_NAME" .
  )

  rm -rf "$TMP_DIR"
  echo "¡Empaquetado completado con éxito!"
}

case "${1:-all}" in
all) compile && compile_tests ;;
compile) compile && compile_tests ;;
clean) clean ;;
entrega) entrega ;;
test) compile && compile_tests && ./$TEST_FILE ;;
*)
  echo "Uso: $0 {all|compile|clean|entrega|test}" >&2
  exit 1
  ;;
esac
