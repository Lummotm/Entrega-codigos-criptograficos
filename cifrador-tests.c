/* Version con flags para pruebas automaticas. Incluye el cifrador real. */
#define main main_cifrador
#include "cifrador.c"
#undef main

int main(int argc, char *argv[]) {
  char *entrada = NULL, *salida = NULL;
  unsigned char clave[14] = "";
  int modo = 0;

  for (int i = 1; i + 1 < argc + 1; i++) {
    if (strcmp(argv[i], "-c") == 0)
      modo = 1;
    else if (strcmp(argv[i], "-d") == 0)
      modo = 2;
    else if (i + 1 < argc && strcmp(argv[i], "-i") == 0)
      entrada = argv[++i];
    else if (i + 1 < argc && strcmp(argv[i], "-o") == 0)
      salida = argv[++i];
    else if (i + 1 < argc && strcmp(argv[i], "-k") == 0)
      strncpy((char *)clave, argv[++i], 13);
  }

  if (!modo || !entrada || !salida || strcmp(entrada, salida) == 0) {
    fprintf(stderr, "Uso: %s -c|-d -i entrada -o salida -k clave\n", argv[0]);
    return 2;
  }
  inicilizar_registro(clave);
  return procesar(modo, entrada, salida);
  return 0;
}
