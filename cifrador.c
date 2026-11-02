#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static short int registro[13] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5};

// Listas con los valores "especiales"
static const unsigned char impares[9] = {5, 7, 9, 11, 13, 15, 17, 19, 21};
static const unsigned char inversos[9] = {205, 183, 57, 163, 197,
                                          239, 241, 27, 61};

// Devuelve el índice de c en impares[], o -1 si no está
// Hago busqueda lineal ya que los arrays son muy pequeños
int obtener_indice(unsigned char c) {
  for (int i = 0; i < 9; i++)
    if (impares[i] == c)
      return i;
  return -1;
}

void avance(void) {
  short int bb, jj;

  bb = 3 * registro[9] + 2 * registro[2] + 5 * registro[0];
  bb = bb % 251;

  for (jj = 0; jj < 12; jj++)
    registro[jj] = registro[jj + 1];

  registro[12] = bb;
}

// Revisa si el archivo existe
int archivo_existe(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file == NULL)
    return 0; // No existe
  fclose(file);
  return 1; // Existe
}

// Limpiar cosas post fgets y scanf
void limpiar(void) {
  if (feof(stdin)) {
    printf("\nEntrada cerrada, saliendo.\n");
    exit(1);
  }
  int hh;
  while ((hh = getchar()) != '\n' && hh != EOF)
    ;
}

void pedir_modo(int *modo) {
  // Pedimos modo
  printf("Que desea hacer?\n");
  printf("1) Cifrar\n2) Descifrar\n");
  printf("Opcion: ");
  scanf("%d", modo);
  limpiar();

  while (*modo != 1 && *modo != 2) {
    printf("\nEscoja una opcion valida (1 o 2): ");
    scanf("%d", modo);
    limpiar();
  }
}

void pedir_ficheros(char *fichero_inicial, char *fichero_final) {

  // Pedimos fichero de entrada
  printf("\nEscriba el nombre del fichero de entrada: ");
  scanf("%299s", fichero_inicial);
  limpiar();

  while (!archivo_existe(fichero_inicial)) {
    printf("\n[ERROR]: El archivo de entrada no existe.");
    printf("\nEscriba el nombre del fichero de entrada: ");
    scanf("%299s", fichero_inicial);
    limpiar();
  }

  // Pedimos fichero de salida
  printf("\nEscriba el nombre del fichero de salida: ");
  scanf("%299s", fichero_final);
  limpiar();

  // Fichero de salida y entrada no pueden ser el mismo (perderiamos ficheros no
  // es buena idea permitir eso)
  while (strcmp(fichero_final, fichero_inicial) == 0) {
    printf("\n[ERROR] Fichero inicial y final no pueden ser los mismos. ");
    printf("\nIntroduce nombre de fichero de salida: ");
    scanf("%299s", fichero_final);
    limpiar();
  }
}
void pedir_clave(unsigned char *clave) {
  printf("\nEscriba la clave (maximo 13 chars): ");
  printf("\nSi la clave es mayor a 13 caracteres se truncara.");

  if (fgets((char *)clave, 14, stdin) != NULL) {
    int len = strlen((char *)clave);
    if (len > 0 && clave[len - 1] == '\n')
      clave[len - 1] = '\0'; // Clave mas corta de 13
    else
      limpiar(); // Clave de más de 13
  } else {
    exit(1);
  }

  printf("\nLa clave es: %s", clave);
}

void inicializar_registro(unsigned char *clave) {
  int j, k;
  for (j = 0; j <= 12; j++) {
    if (clave[j] == '\0' || clave[j] == '\n')
      break;
    registro[j] = (short)clave[j] % 251;
  }
  k = registro[3];

  for (j = 0; j < k; j++)
    avance();
}

int procesar(int modo, char *fichero_inicial, char *fichero_final) {
  // La asignacion a unsigned char reduce el resultado modulo 256
  unsigned char leocaracter, escribocaracter, cifrante;

  // Abrir fichero de entrada
  FILE *fientrada;
  fientrada = fopen(fichero_inicial, "rb");
  if (fientrada == NULL) {
    printf("\n[ERROR]: no he podido abrir el fichero de entrada");
    return 1;
  }

  // Abrir fichero de salida
  FILE *fisalida;
  fisalida = fopen(fichero_final, "wb");
  if (fisalida == NULL) {
    fclose(fientrada);
    printf("\n[ERROR]: no he podido abrir el fichero de salida");
    return 1;
  }

  // fread devuelve 1 si ha leido un byte correctamente
  while (fread(&leocaracter, sizeof(leocaracter), 1, fientrada) == 1) {
    cifrante = (unsigned char)registro[12];

    // Cifrante par: suma. Cifrante impar de la lista: producto. Resto: XOR

    if (cifrante % 2 == 0) {
      // e_n = m_n + c_n (cifrar) y  m_n = e_n - c_n (descifrar)
      if (modo == 1)
        escribocaracter = leocaracter + cifrante;
      else
        escribocaracter = leocaracter - cifrante;

    } else {
      int indice = obtener_indice(cifrante);

      if (indice != -1) {
        // Producto por el impar (cifrar) o su inverso (descifrar)

        if (modo == 1)
          escribocaracter = leocaracter * cifrante;
        else
          escribocaracter = leocaracter * inversos[indice];

      } else {
        // Suma bit a bit (XOR): igual para cifrar y descifrar
        escribocaracter = leocaracter ^ cifrante;
      }
    }

    fwrite(&escribocaracter, sizeof(escribocaracter), 1, fisalida);
    avance();
  }

  fclose(fisalida);
  fclose(fientrada);

  return 0;
}

int main(void) {
  char fichero_inicial[300] = "";
  char fichero_final[300] = "";
  unsigned char clave[14] = "";
  int modo = 0;

  // Parsing de los datos
  pedir_modo(&modo);
  pedir_ficheros(fichero_inicial, fichero_final);
  pedir_clave(clave);

  // Inicializado del registro
  inicializar_registro(clave);

  // Procesado, si funciona bien devuelve 0 si falla 1
  return procesar(modo, fichero_inicial, fichero_final);
}
