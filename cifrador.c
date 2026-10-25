/* Programa de cifrajo en flujo basico y sencillo;
un intento pedagogico y para recordar algo la programacion en C*/

#include <stdio.h>
#include <stdlib.h> // hace falta para el exit(0)
#include <string.h>

/*Variable global que representa el vector de estado de la sucesion cifrante*/
/*Es menos seguro el uso de variables globales, pero mas sencillo*/
static short int registro[13] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5};

static const unsigned char impares[9] = {5, 7, 9, 11, 13, 15, 17, 19, 21};
static const unsigned char inversos[9] = {205, 183, 57, 163, 197,
                                          239, 241, 27, 61};

int get_index(unsigned char c);
void print_help(char *error_msg);
void avance(void);
void clean_buffer(void);
int file_exists(const char *filename);
void check_flags(int argc, char *argv[], int *modo, char *fichero_inicial,
                 char *fichero_final, unsigned char *clave);
void ask_missing_data(int *modo, char *fichero_inicial, char *fichero_final,
                      unsigned char *clave);


int main(int argc, char *argv[]) {
  char fichero_inicial[300] = "";
  char fichero_final[300] = "";
  unsigned char clave[14] = "";
  unsigned char leocaracter, escribocaracter, cifrante;

  int j, k, modo = 0;

  // Parsing de los datos
  check_flags(argc, argv, &modo, fichero_inicial, fichero_final, clave);
  ask_missing_data(&modo, fichero_inicial, fichero_final, clave);

  for (j = 0; j <= 12; j++) {
    if (clave[j] == '\0' || clave[j] == '\n')
      break;
    registro[j] = (short)clave[j] % 251;
  }
  k = registro[3];

  for (j = 0; j < k; j++)
    avance();

  // Abrir fichero de entrada (se checkea que existe en el
  // parsing de argumentos ya.)
  FILE *fientrada;
  fientrada = fopen(fichero_inicial, "rb");

  /*Abrimos el fichero de salida que estara encriptado*/
  FILE *fisalida;
  fisalida = fopen(fichero_final, "wb");

  if (fisalida == NULL) {
    printf("[ERROR]: no he podido abrir el fichero de salida");
    return 1;
  }

  fread(&leocaracter, sizeof(leocaracter), 1, fientrada);
  while (feof(fientrada) == 0) {
    cifrante = (unsigned char)registro[12];

    // escribocaracter = leocaracter ^ cifrante; /*suma binaria XOR */
    // Para el problema 18 tenemos que cambiar esta lógica.

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
    fread(&leocaracter, sizeof(leocaracter), 1, fientrada);
  }
  fclose(fisalida);
  fclose(fientrada);

  return 0;
}

// Devuelve el índice de c en impares[], o -1 si no está
// Podria haber hecho algo mas rapido una busqueda bianaria o algo asi pero no
// le veo sentido
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

void clean_buffer(void) {
  int hh;
  while ((hh = getchar()) != '\n' && hh != EOF)
    ;
}


int file_exists(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file != NULL) {
    fclose(file);
    return 0; // File exists
  }
  return 1;
}


/* Funcion para pedir los datos faltantes interactivamente */
void ask_missing_data(int *modo, char *fichero_inicial, char *fichero_final,
                      unsigned char *clave) {
  // Revisamos si el argumento ya ha sido cambiado por los flags

  if (*modo != 1 && *modo != 2) {
    printf("Que desea hacer?\n");
    printf("1) Cifrar\n2) Descifrar\n");
    printf("Opcion: ");
    scanf("%d", modo);

    while (*modo != 1 && *modo != 2) {
      printf("Escoja una opcion valida (1 o 2): ");
      scanf("%d", modo);
    }

    clean_buffer();
  }

  if (strlen(fichero_inicial) == 0) {
    printf("\nEscriba el nombre del fichero de entrada: ");
    scanf("%299s", fichero_inicial);

    while (file_exists(fichero_inicial) == 1) {
      printf("[ERROR]: El archivo de entrada no existe.\n");
      printf("Escriba el nombre del fichero de entrada: ");
      scanf("%299s", fichero_inicial);
    }

    clean_buffer();
  }

  if (strlen(fichero_final) == 0) {
    printf("\nEscriba el nombre del fichero de salida: ");
    scanf("%299s", fichero_final);
    clean_buffer();
    while (strcmp(fichero_final, fichero_inicial) == 0) {
      printf("\n[ERROR] Fichero inicial y final no pueden ser los mismos. ");
      printf("\nIntroduce nombre de fichero de salida: ");
      scanf("%299s", fichero_final);
      clean_buffer();
    }
  }

  if (strlen((char *)clave) == 0) {
    printf("\nEscriba la clave (maximo 13 chars): ");
    printf("\nSi la clave es mayor a 13 chars se truncara. ");
    if (fgets((char *)clave, 14, stdin) != NULL) {
      int len = strlen((char *)clave);
      if (len > 0 && clave[len - 1] == '\n')
        clave[len - 1] = '\0';
      else
        clean_buffer();
    }
  }
  printf("\nLa clave es: %s", clave);
}


/* Parseo de flags por línea de comandos
 * Usado para poder hacer tests por ejemplo con urandom y demas, revisar que
 * bajo cualquier condicion encripta y desencripta bien
 * */
void check_flags(int argc, char *argv[], int *modo, char *fichero_inicial,
                 char *fichero_final, unsigned char *clave) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--cifrar") == 0 || strcmp(argv[i], "-c") == 0)
      *modo = 1;

    else if (strcmp(argv[i], "--descifrar") == 0 || strcmp(argv[i], "-d") == 0)
      *modo = 2;

    else if ((strcmp(argv[i], "--input") == 0 || strcmp(argv[i], "-i") == 0) &&
             i + 1 < argc) {

      if (argv[i + 1][0] == '-')
        print_help("El nombre del archivo de input no es valido.");

      if (file_exists(argv[i + 1]) == 1)
        print_help("El archivo de input no existe.");

      strncpy(fichero_inicial, argv[i + 1], 299);
      fichero_inicial[299] = '\0';
      i++;

    } else if ((strcmp(argv[i], "--output") == 0 ||
                strcmp(argv[i], "-o") == 0) &&
               i + 1 < argc) {
      if (argv[i + 1][0] == '-')
        print_help("El nombre del archivo de output no es valido.");

      strncpy(fichero_final, argv[i + 1], 299);
      fichero_final[299] = '\0';
      i++;

    } else if ((strcmp(argv[i], "--clave") == 0 ||
                strcmp(argv[i], "-k") == 0) &&
               i + 1 < argc) {
      strncpy((char *)clave, argv[i + 1], 13);
      clave[13] = '\0';
      i++;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      print_help("");
    }
  }
}

void print_help(char *error_msg) {
  int error_code = 0;
  if (strlen((const char *)error_msg) != 0) {
    printf("\n[ERROR]: %s\n", error_msg);
    error_code = 1;
  }
  printf("Uso:\n");
  printf("  ./cifrador [OPCIONES]\n\n");
  printf("Opciones disponibles:\n");
  printf("  -c, --cifrar            Indica que se desea cifrar un archivo\n");
  printf(
      "  -d, --descifrar         Indica que se desea descifrar un archivo\n");
  printf("  -i, --input <fichero>   Fichero de entrada\n");
  printf("  -o, --output <fichero>  Fichero de salida\n");
  printf("  -k, --clave <clave>     Clave secreta (maximo 13 caracteres)\n");
  printf("  -h, --help              Muestra esta ayuda y sale\n\n");
  printf("Ejemplos:\n");
  printf(
      "  ./cifrador --cifrar -i texto.txt -o cifrado.bin -c miClaveSecreta\n");
  printf("  ./cifrador (modo interactivo: pedira los datos por consola)\n\n");
  exit(error_code);
}
