#!/usr/bin/env bash

FILE1="files/a_cifrar"
FILE2="files/cifrado"
FILE3="files/descifrado"
KEY="11234156"

mkdir -p files

fail=0

# Ejecuta un comando y devuelve por pantalla el tiempo en segundos
medir() {
  local t0 t1
  t0=$(date +%s.%N)
  "$@" >/dev/null || return 1
  t1=$(date +%s.%N)
  awk "BEGIN {printf \"%.2f\", $t1 - $t0}"
}

test() {
  local desc="$1" t_cif t_des

  if ! t_cif=$(medir ./main_tests -c -i "$FILE1" -o "$FILE2" -k "$KEY"); then
    echo " FAIL (cifrado)  $desc"
    fail=1
    return
  fi

  if ! t_des=$(medir ./main_tests -d -i "$FILE2" -o "$FILE3" -k "$KEY"); then
    echo " FAIL (descifrado) $desc"
    fail=1
    return
  fi

  if cmp -s "$FILE1" "$FILE3"; then
    echo " OK   $desc  (cifrado ${t_cif}s, descifrado ${t_des}s)"
  else
    echo " FAIL (no coinciden) $desc"
    fail=1
  fi
}

# Test con todo ceros
head -c 1024 /dev/zero >"$FILE1"
test "todo ceros (1024 bytes)"

# Test con todo 0xFF
head -c 1024 /dev/zero | tr '\0' '\377' >"$FILE1"
test "todo 0xFF (1024 bytes)"

# Tests con urandom

SIZE=10485760 # 10 MB
head -c $SIZE /dev/urandom >"$FILE1"
READABLE="$(awk "BEGIN {print $SIZE / 1024 / 1024}") MB"
test "urandom $READABLE bytes"

SIZE=$((10485760 * 5)) # 50 MB
head -c $SIZE /dev/urandom >"$FILE1"
READABLE="$(awk "BEGIN {print $SIZE / 1024 / 1024}") MB"
test "urandom $READABLE bytes"

SIZE=104857600 # 100 MB
head -c $SIZE /dev/urandom >"$FILE1"
READABLE="$(awk "BEGIN {print $SIZE / 1024 / 1024}") MB"
test "urandom $READABLE bytes"

SIZE=$((104857600 * 5)) # 500 MB
head -c $SIZE /dev/urandom >"$FILE1"
READABLE="$(awk "BEGIN {print $SIZE / 1024 / 1024}") MB"
test "urandom $READABLE bytes"

SIZE=1048576000 # 1000 MB
head -c $SIZE /dev/urandom >"$FILE1"
READABLE="$(awk "BEGIN {print $SIZE / 1024 / 1024}") MB"
test "urandom $READABLE bytes"

if [ "$fail" -eq 0 ]; then
  echo
  echo "Todos los tests han pasado"
else
  echo
  echo "Algun test ha fallado"
fi

exit $fail
