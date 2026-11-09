#!/usr/bin/env bash
# Pruebas del cifrador. Recompila main_tests desde cifrador-tests.c y lo
# compara con la referencia independiente ref.py. Ademas comprueba ida y
# vuelta en ficheros grandes.

cd "$(dirname "$0")" || exit 1

SRC=$HOME/Projects/univ/Entrega-codigos-criptograficos/cifrador-tests.c
PROG=$HOME/Projects/univ/Entrega-codigos-criptograficos/main_tests
REF=$HOME/Projects/univ/Entrega-codigos-criptograficos/utils/ref.py
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
[ -f "$SRC" ] || {
  echo "Falta $SRC"
  exit 1
}
[ -f "$REF" ] || {
  echo "Falta $REF"
  exit 1
}

# Recompilar siempre antes de probar
echo "Compilando $PROG desde $SRC..."
if ! gcc -std=c99 -Wall -Wextra -O2 "$SRC" -o "$PROG"; then
  echo "Error de compilacion, no se ejecutan las pruebas"
  exit 1
fi
echo

TMP=$(mktemp -d) || exit 1
trap 'rm -rf "$TMP"' EXIT

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

  ref -d -i "$c" -k "$key" | cmp -s - "$in" ||
    {
      ko "(descifrado referencia) $desc"
      return
    }
  ok "$desc"
}

# Cifra y descifra con el programa C y mide el tiempo. Solo ficheros grandes.
probar_ida_vuelta() {
  local desc=$1 in=$2
  local c="$TMP/c.bin" d="$TMP/d.bin" t0 t1

  t0=$(date +%s.%N)
  prog -c -i "$in" -o "$c" -k "$KEY" && prog -d -i "$c" -o "$d" -k "$KEY" ||
    {
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

# Clave incorrecta
probar_clave_incorrecta "clave incorrecta no recupera el original" "$ALEATORIO"

# Ficheros grandes: solo ida y vuelta (la referencia en Python es lenta)
for mb in 10 50; do
  head -c $((mb * 1024 * 1024)) /dev/urandom >"$TMP/grande.bin"
  probar_ida_vuelta "urandom ${mb} MB" "$TMP/grande.bin"
done

printf '\x01\x01\x01' >"$TMP/v_in.bin"
printf '\x05\x07\x09' >"$TMP/v_esp.bin"
prog -c -i "$TMP/v_in.bin" -o "$TMP/v_out.bin" -k ""
if cmp -s "$TMP/v_out.bin" "$TMP/v_esp.bin"; then
  ok "vector a mano (clave vacia)"
else
  ko "vector a mano (clave vacia)"
fi

printf '\x01\x01\x01\x01\x01\x01\x01' >"$TMP/v7_in.bin"
printf '\x05\x07\x09\x0d\x0f\x13\x1a' >"$TMP/v7_esp.bin"
prog -c -i "$TMP/v7_in.bin" -o "$TMP/v7_out.bin" -k ""
if cmp -s "$TMP/v7_out.bin" "$TMP/v7_esp.bin"; then
  ok "vector a mano de 7 bytes (3 ramas, clave vacia)"
else
  ko "vector a mano de 7 bytes (3 ramas, clave vacia)"
fi

echo
if [ "$fail" -eq 0 ]; then
  echo "Todos los tests han pasado"
else
  echo "Algun test ha fallado"
fi
exit $fail
