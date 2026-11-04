/* Version con flags para pruebas automaticas. Incluye el cifrador real. */
#define main main_cifrador
#include "cifrador.c"
#undef main

int main(int argc, char *argv[]) {
  char *fichero_inicial = NULL, *fichero_final = NULL;
  unsigned char clave[14] = "";
  int modo = 0;

  for (int i = 1; i + 1 < argc + 1; i++) {
    if (strcmp(argv[i], "-c") == 0)
      modo = 1;
    else if (strcmp(argv[i], "-d") == 0)
      modo = 2;
    else if (i + 1 < argc && strcmp(argv[i], "-i") == 0)
      fichero_inicial = argv[++i];
    else if (i + 1 < argc && strcmp(argv[i], "-o") == 0)
      fichero_final = argv[++i];
    else if (i + 1 < argc && strcmp(argv[i], "-k") == 0)
      strncpy((char *)clave, argv[++i], 13);
  }

  if (!modo || !fichero_inicial || !fichero_final ||
      strcmp(fichero_inicial, fichero_final) == 0) {
    fprintf(stderr,
            "Uso: %s -c|-d -i fichero_inicial -o fichero_final -k clave\n",
            argv[0]);
    return 2;
  }
  inicializar_registro(clave);

  // Trabajamos con los modos
  if (modo == 1) {
    return cifrar(fichero_inicial, fichero_final);
  } else if (modo == 2) {
    return descifrar(fichero_inicial, fichero_final);
  }
}
