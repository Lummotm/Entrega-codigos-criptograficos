#!/usr/bin/env bash
set -uo pipefail

TARGET="main"
SRC="cifrador.c"
AUTHOR_FILE="utils/author.txt"
ARCHIVO_ENTREGA="NikolovYordanovDavid_Ejercicio_18.c"
TEST_SRC="cifrador-tests.c"
TEST_BIN="main_tests"
TEST_FILE="utils/test.sh"
ZIP_NAME="entrega_cripto.zip"
CC="gcc"
CFLAGS="-Wall -Wextra -O2 "
TMP_DIR="tmp_entrega"

# Compilar
compile() { "$CC" $CFLAGS "$SRC" -o "$TARGET"; }

compile_tests() { "$CC" $CFLAGS "$TEST_SRC" -o "$TEST_BIN"; }

# Limpiar
clean() {
  rm -f "$TARGET" "$ZIP_NAME" *~ *.c_entrega || True
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
  } >"$ARCHIVO_ENTREGA"

  echo "Archivo creado con exito"
}

test_all() {
  local fail=0
  for cc in gcc clang; do
    if ! command -v "$cc" >/dev/null; then
      echo "$cc no instalado, lo omito"
      continue
    fi
    echo "== $cc =="
    if "$cc" $CFLAGS "$TEST_SRC" -o "${TEST_BIN}_$cc"; then
      TEST_BIN="./${TEST_BIN}_$cc" ./$TEST_FILE || fail=1
    else
      echo "FAIL (compilacion con $cc)"
      fail=1
    fi
  done
  return $fail
}

case "${1:-all}" in
all) compile && compile_tests ;;
compile) compile && compile_tests ;;
clean) clean ;;
entrega) entrega ;;
test) compile && compile_tests && ./$TEST_FILE ;;
test_all) test_all ;;
*)
  echo "Uso: $0 {all|compile|clean|entrega|test}" >&2
  exit 1
  ;;
esac
