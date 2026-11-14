#!/usr/bin/env bash
# Pruebas del cifrador. Recompila el programa de pruebas desde cifrador-tests.c
# y lo compara con la referencia independiente ref.py. Ademas comprueba ida y
# vuelta en ficheros grandes y el programa interactivo (./main), incluido el
# manejo de entrada invalida y de Ctrl+D.
# Todos los ficheros de prueba se crean en un directorio temporal dentro de
# files/ y se borran al final.

DIR=$(cd "$(dirname "$0")" && pwd) || exit 1
ROOT=$(cd "$DIR/.." && pwd) || exit 1
FILES=$ROOT/files
SRC=$ROOT/cifrador-tests.c
REF=$DIR/ref.py
MAIN=$ROOT/main
KEY="11234156"

# Comprobaciones previas
command -v gcc >/dev/null || {
  echo "Hace falta gcc"
  exit 1
}
command -v python3 >/dev/null || {
  echo "Hace falta python3"
  exit 1
}
command -v timeout >/dev/null || {
  echo "Hace falta timeout (coreutils)"
  exit 1
}
[ -f "$SRC" ] || {
  echo "Falta $SRC"
  exit 1
}
[ -f "$REF" ] || {
  echo "Falta $REF"
  exit 1
}
[ -x "$MAIN" ] || {
  echo "Falta $MAIN (compila el programa interactivo primero)"
  exit 1
}
[ -d "$FILES" ] || {
  echo "Falta el directorio $FILES"
  exit 1
}

TMP=$(mktemp -d "$FILES/.tests.XXXXXX") || exit 1
trap 'rm -rf "$TMP"' EXIT

PROG=$TMP/main_tests

echo "Compilando $PROG desde $SRC..."
if ! gcc -std=c99 -Wall -Wextra -O2 "$SRC" -o "$PROG"; then
  echo "Error de compilacion, no se ejecutan las pruebas"
  exit 1
fi
echo

fail=0
ok() { echo " OK   $1"; }
ko() {
  echo " FAIL $1"
  fail=1
}

prog() { "$PROG" "$@" >/dev/null; }
ref() { python3 "$REF" "$@"; }

# Compara el programa con la referencia en ambos sentidos.
# $1 descripcion, $2 fichero de entrada, $3 clave
probar_referencia() {
  local desc=$1 in=$2 key=$3
  local c="$TMP/c.bin" r="$TMP/r.bin" d="$TMP/d.bin"

  prog -c -i "$in" -o "$c" -k "$key" || {
    ko "(cifrado C) $desc"
    return
  }
  ref -c -i "$in" -k "$key" >"$r" || {
    ko "(referencia cifrado) $desc"
    return
  }
  cmp -s "$c" "$r" || {
    ko "(cifrado distinto de la referencia) $desc"
    return
  }

  prog -d -i "$c" -o "$d" -k "$key" || {
    ko "(descifrado C) $desc"
    return
  }
  cmp -s "$in" "$d" || {
    ko "(descifrado C no recupera el original) $desc"
    return
  }

  ref -d -i "$c" -k "$key" >"$d" || {
    ko "(descifrado referencia) $desc"
    return
  }
  cmp -s "$in" "$d" || {
    ko "(descifrado referencia no recupera el original) $desc"
    return
  }

  ok "$desc"
}

# Ida y vuelta con el programa C, con medida de tiempo. Solo ficheros grandes.
probar_ida_vuelta() {
  local desc=$1 in=$2
  local c="$TMP/c.bin" d="$TMP/d.bin" t0 t1

  t0=$(date +%s.%N)
  prog -c -i "$in" -o "$c" -k "$KEY" && prog -d -i "$c" -o "$d" -k "$KEY" || {
    ko "(ejecucion) $desc"
    return
  }
  t1=$(date +%s.%N)

  if cmp -s "$in" "$d"; then
    ok "$desc (ida y vuelta $(awk "BEGIN {printf \"%.2f\", $t1 - $t0}")s)"
  else
    ko "$desc"
  fi
}

# Con clave incorrecta el descifrado no debe recuperar el original.
probar_clave_incorrecta() {
  local desc=$1 in=$2
  local c="$TMP/c.bin" d="$TMP/d.bin"

  prog -c -i "$in" -o "$c" -k "$KEY" || {
    ko "(ejecucion) $desc"
    return
  }
  prog -d -i "$c" -o "$d" -k "clave_erronea" || {
    ko "(ejecucion) $desc"
    return
  }

  if cmp -s "$in" "$d"; then ko "$desc"; else ok "$desc"; fi
}

# Ejecuta ./main con la entrada dada y comprueba codigo de salida y texto.
# $1 descripcion, $2 entrada (escapes interpretados por printf %b),
# $3 texto que debe aparecer en la salida ("" = no comprobar),
# $4 codigo de salida esperado
probar_interactivo() {
  local desc=$1 entrada=$2 esperado=$3 rc_esp=$4
  local out="$TMP/salida_main.txt" rc

  printf '%b' "$entrada" | timeout 5 "$MAIN" >"$out" 2>&1
  rc=$?

  if [ "$rc" -eq 124 ]; then
    ko "(bucle infinito, timeout) $desc"
  elif [ "$rc" -ne "$rc_esp" ]; then
    ko "(codigo $rc, esperado $rc_esp) $desc"
  elif [ -n "$esperado" ] && ! grep -qF -- "$esperado" "$out"; then
    ko "(no aparece '$esperado') $desc"
  else
    ok "$desc"
  fi
}

# Ficheros de prueba
VACIO="$TMP/vacio.bin"
: >"$VACIO"

TODOS_CEROS="$TMP/ceros.bin"
head -c 1024 /dev/zero >"$TODOS_CEROS"

TODOS_FF="$TMP/ff.bin"
head -c 1024 /dev/zero | tr '\0' '\377' >"$TODOS_FF"

VALORES="$TMP/valores.bin"
python3 -c 'import sys; sys.stdout.buffer.write(bytes(range(256)))' >"$VALORES"

ALEATORIO="$TMP/aleatorio_1m.bin"
head -c 1048576 /dev/urandom >"$ALEATORIO"

PRUEBA="$TMP/prueba.txt"
printf 'prueba' >"$PRUEBA"

# Casos pequeños contra la referencia
probar_referencia "fichero vacio" "$VACIO" "$KEY"
probar_referencia "todo ceros (1024 bytes)" "$TODOS_CEROS" "$KEY"
probar_referencia "todo 0xFF (1024 bytes)" "$TODOS_FF" "$KEY"
probar_referencia "los 256 valores de byte" "$VALORES" "$KEY"
probar_referencia "urandom 1 MB" "$ALEATORIO" "$KEY"

# Claves limite contra la referencia
probar_referencia "clave vacia" "$ALEATORIO" ""
probar_referencia "clave de 1 caracter" "$ALEATORIO" "a"
probar_referencia "clave de 13 caracteres" "$ALEATORIO" "abcdefghijklm"
probar_referencia "clave de mas de 13 (se trunca)" "$ALEATORIO" "abcdefghijklmnopq"
probar_referencia "clave con no ASCII y espacios" "$ALEATORIO" "ñandú 2"

# Clave incorrecta
probar_clave_incorrecta "clave incorrecta no recupera el original" "$ALEATORIO"

# Ficheros grandes: solo ida y vuelta (la referencia en Python es lenta)
for mb in 10 50; do
  head -c $((mb * 1024 * 1024)) /dev/urandom >"$TMP/grande.bin"
  probar_ida_vuelta "urandom ${mb} MB" "$TMP/grande.bin"
done

# Vectores a mano (clave vacia)
printf '\x01\x01\x01' >"$TMP/v_in.bin"
printf '\x05\x07\x09' >"$TMP/v_esp.bin"
if prog -c -i "$TMP/v_in.bin" -o "$TMP/v_out.bin" -k "" &&
  cmp -s "$TMP/v_out.bin" "$TMP/v_esp.bin"; then
  ok "vector a mano (clave vacia)"
else
  ko "vector a mano (clave vacia)"
fi

printf '\x01\x01\x01\x01\x01\x01\x01' >"$TMP/v7_in.bin"
printf '\x05\x07\x09\x0d\x0f\x13\x1a' >"$TMP/v7_esp.bin"
if prog -c -i "$TMP/v7_in.bin" -o "$TMP/v7_out.bin" -k "" &&
  cmp -s "$TMP/v7_out.bin" "$TMP/v7_esp.bin"; then
  ok "vector a mano de 7 bytes (3 ramas, clave vacia)"
else
  ko "vector a mano de 7 bytes (3 ramas, clave vacia)"
fi

# Programa interactivo (./main). Todos sus ficheros van en $TMP.

# Ctrl+D al inicio: debe avisar y salir con codigo 1
probar_interactivo "Ctrl+D al inicio" "" "Entrada cerrada" 1

# Opcion invalida (letra, numero fuera de rango) y luego valida
probar_interactivo "opcion invalida antes de la valida" \
  "x\n3\n1\n$PRUEBA\n$TMP/o1.cif\n\n" "Escoja una opcion valida" 0

# Fichero de entrada inexistente y luego uno valido
probar_interactivo "fichero de entrada inexistente y luego valido" \
  "1\n$TMP/no_existe.txt\n$PRUEBA\n$TMP/o2.cif\n\n" "no existe" 0

# Salida igual a entrada: debe rechazarla
probar_interactivo "salida igual a entrada se rechaza" \
  "1\n$PRUEBA\n$PRUEBA\n$TMP/o3.cif\n\n" "no pueden ser los mismos" 0

# Directorio de salida inexistente: error de apertura
probar_interactivo "salida en directorio inexistente" \
  "1\n$PRUEBA\n$TMP/no/existe.cif\n\n" "no he podido abrir el fichero de salida" 1

# Ctrl+D al pedir la salida o la clave: sin bucle infinito
probar_interactivo "Ctrl+D al pedir la salida" "1\n$PRUEBA\n" "" 1
probar_interactivo "Ctrl+D al pedir la clave" "1\n$PRUEBA\n$TMP/o4.cif\n" "" 1

# Clave larga y clave de 13 caracteres: el resultado debe ser igual (se trunca)
probar_interactivo "cifrado con clave larga" \
  "1\n$PRUEBA\n$TMP/largo.cif\nabcdefghijklmnopq\n" "" 0
probar_interactivo "cifrado con clave de 13 caracteres" \
  "1\n$PRUEBA\n$TMP/corto.cif\nabcdefghijklm\n" "" 0
if cmp -s "$TMP/largo.cif" "$TMP/corto.cif"; then
  ok "clave truncada a 13 caracteres (interactivo)"
else
  ko "clave truncada a 13 caracteres (interactivo)"
fi

echo
if [ "$fail" -eq 0 ]; then
  echo "Todos los tests han pasado"
else
  echo "Algun test ha fallado"
fi
exit $fail
