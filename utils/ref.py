#!/usr/bin/env python3
"""Referencia independiente del cifrador, solo para pruebas.

Uso: ref.py -c|-d -i FICHERO -k CLAVE > SALIDA
"""

import sys

INVERSOS = {5: 205, 7: 183, 9: 57, 11: 163, 13: 197, 15: 239, 17: 241, 19: 27, 21: 61}


def avanzar(r):
    """Avanza el registro un paso (generador de la sucesion cifrante, mod 251)."""
    bb = (3 * r[9] + 2 * r[2] + 5 * r[0]) % 251
    r.pop(0)
    r.append(bb)


def procesar(datos, clave, cifrando):
    """Aplica la operacion del enunciado a cada byte de datos."""
    r = [0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5]
    for j, ch in enumerate(clave[:13]):
        r[j] = ch % 251
    for _ in range(r[3]):
        avanzar(r)

    salida = bytearray()
    for m in datos:
        c = r[12]
        if c % 2 == 0:
            e = (m + c) % 256 if cifrando else (m - c) % 256
        elif 5 <= c <= 21:
            e = (m * c) % 256 if cifrando else (m * INVERSOS[c]) % 256
        else:
            e = m ^ c
        salida.append(e)
        avanzar(r)
    return bytes(salida)


def main(argv):
    if (
        len(argv) != 6
        or argv[1] not in ("-c", "-d")
        or argv[2] != "-i"
        or argv[4] != "-k"
    ):
        sys.stderr.write("Uso: ref.py -c|-d -i FICHERO -k CLAVE\n")
        return 2
    cifrando = argv[1] == "-c"
    with open(argv[3], "rb") as f:
        datos = f.read()
    clave = argv[5].encode()
    sys.stdout.buffer.write(procesar(datos, clave, cifrando))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
