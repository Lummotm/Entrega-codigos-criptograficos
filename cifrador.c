/* Programa de cifrajo en flujo basico y sencillo;
   un intento pedagogico y para recordar algo la programacion en C*/

#include <stdio.h>
#include <string.h>

/*Variable global que representa el vector de estado de la sucesion cifrante*/
/*Es menos seguro el uso de variables globales, pero mas sencillo*/
static short int registro[13] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5};

/*Listas con los valores "especiales"*/
static const unsigned char impares[9] = {5, 7, 9, 11, 13, 15, 17, 19, 21};
static const unsigned char inversos[9] = {205, 183, 57, 163, 197,
                                          239, 241, 27, 61};

// Devuelve el índice de c en impares[], o -1 si no está
// Podria haber hecho algo mas rapido una busqueda bianaria o algo asi pero no
// le veo sentido, en una lista tan pequeña podemos simplemente hacerlo lineal
int get_index(unsigned char c) {
  for (int i = 0; i < 9; i++)
    if (impares[i] == c)
      return i;
  return -1;
}

/* Este subprograma hace que el registro avance un ciclo completo*/
void avance(void) {
  short int bb, jj;

  bb = 3 * registro[9] + 2 * registro[2] + 5 * registro[0];
  bb = bb % 251;

  for (jj = 0; jj < 12; jj++)
    registro[jj] = registro[jj + 1];

  registro[12] = bb;
}

/* Limpiar despues de leer */
void limpiar(void) {
  int hh;
  while ((hh = getchar()) != '\n' && hh != EOF)
    ;
}

/* Revisa si el archivo existe*/
int archivo_existe(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file != NULL) {
    fclose(file);
    return 0; // Existe
  }
  return 1; // No existe
}

/* Función para pedir los datos*/
void pedir_datos(int *modo, char *fichero_inicial, char *fichero_final,
                 unsigned char *clave) {

  // Pedimos modo
  printf("Que desea hacer?\n");
  printf("1) Cifrar\n2) Descifrar\n");
  printf("Opcion: ");
  scanf("%d", modo);

  while (*modo != 1 && *modo != 2) {
    printf("Escoja una opcion valida (1 o 2): ");
    scanf("%d", modo);
  }
  limpiar();

  // Pedimos fichero de entrada
  printf("\nEscriba el nombre del fichero de entrada: ");
  scanf("%299s", fichero_inicial);

  // Verificamos que existe
  // Devuelve 1 en fallo luego simplmenete si pasa es que hay fallo
  while (archivo_existe(fichero_inicial)) {
    printf("[ERROR]: El archivo de entrada no existe.\n");
    printf("Escriba el nombre del fichero de entrada: ");
    scanf("%299s", fichero_inicial);
  }
  limpiar();

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

  // Pedimos clave
  printf("\nEscriba la clave (maximo 13 chars): ");
  // Avisamos de la clave se trunca a los 13 chars
  printf("\nSi la clave es mayor a 13 chars se truncara. ");
  if (fgets((char *)clave, 14, stdin) != NULL) {
    int len = strlen((char *)clave);
    if (len > 0 && clave[len - 1] == '\n')
      clave[len - 1] = '\0'; // Clave mas corta de 13
    else
      limpiar(); // Clave de más de 13
  }

  printf("\nLa clave es: %s", clave);
}

void inicilizar_registro(unsigned char *clave) {
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
  unsigned char leocaracter, escribocaracter, cifrante;

  // Abrir fichero de entrada
  FILE *fientrada;
  fientrada = fopen(fichero_inicial, "rb");
  if (fientrada == NULL) {
    fclose(fientrada);
    printf("[ERROR]: no he podido abrir el fichero de entrada");
    return 1;
  }

  // Abrir fichero de salida
  FILE *fisalida;
  fisalida = fopen(fichero_final, "wb");

  if (fisalida == NULL) {
    printf("[ERROR]: no he podido abrir el fichero de salida");
    return 1;
  }

  // Uso fread == 1 pq como en este caso leemos 1 bit si es correcta la lectura
  // entonces obtenemos 1 bit
  /*
    Del manual de C.
    On success, fread() and fwrite() return the number of items read or written.
    This number equals the number of bytes transferred only when size is 1. If
    an error occurs, or the end of the file is reached, the return value is a
    short item count (or zero).
  */
  while (fread(&leocaracter, sizeof(leocaracter), 1, fientrada) == 1) {
    cifrante = (unsigned char)registro[12];

    // Dado el valor de la sucesión cifrante buscamos si esta entre los impares
    // con los que trabajamos si esta, entonces guardamos su indice, si estamos
    // cifrando y tenemos indice entonces trabajamos con el caso de impar, si
    // estamos descifrando hacemos lo mismo pero en vez de trabajar con el valor
    // impar trabajamos con su inverso

    int idx = get_index(cifrante);
    if (cifrante % 2 == 0) {
      if (modo == 1) // e_n = m_n + c_n
        escribocaracter = (leocaracter + cifrante) % 256;
      else
        // e_n - m_n = c_n
        // En principio no deberia de haber problema de negatividad
        // estamos trabajando con unsigned char justamente por eso
        // Añado el + 256 para evitar ese fallo
        escribocaracter = (leocaracter - cifrante + 256) % 256;
    } else if (idx != -1) {
      // Producto modulo 256 para los casos especificos de cn
      if (modo == 1)
        escribocaracter = (leocaracter * cifrante) % 256;
      else
        escribocaracter = (leocaracter * inversos[idx]) % 256;
    } else {
      // Suma XOR
      escribocaracter = leocaracter ^ cifrante;
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
  pedir_datos(&modo, fichero_inicial, fichero_final, clave);

  // Inicializado del registro
  inicilizar_registro(clave);

  // Procesado, si funciona bien devuelve 0 si falla 1
  return procesar(modo, fichero_inicial, fichero_final);
}
