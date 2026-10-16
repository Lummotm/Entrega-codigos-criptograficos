/* Programa de cifrajo en flujo basico y sencillo;
un intento pedagogico y para recordar algo la programacion en C*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h> // hace falta para el exit(0)


/*Variable global que representa el vector de estado de la sucesion cifrante*/
/*Es menos seguro el uso de variables globales, pero mas sencillo*/
static short int registro[13] = {0,0,0,0,0,0,0,0,1,2,3,4,5};


// Igual demasiadas funciones auxiliares?? 
void clean_buffer(void) {
  int hh;
  while ((hh = getchar()) != '\n' && hh != EOF);
}

void print_help(char *error_msg) {
  if (strlen((const char *)error_msg) != 0) printf("\n[ERROR]: %s\n",error_msg);
  printf("Uso:\n");
  printf("  ./cifrador [OPCIONES]\n\n");
  printf("Opciones disponibles:\n");
  printf("  -c, --cifrar            Indica que se desea cifrar un archivo\n");
  printf("  -d, --descifrar         Indica que se desea descifrar un archivo\n");
  printf("  -i, --input <fichero>   Fichero de entrada\n");
  printf("  -o, --output <fichero>  Fichero de salida\n");
  printf("  -k, --clave <clave>     Clave secreta (maximo 13 caracteres)\n");
  printf("  -h, --help              Muestra esta ayuda y sale\n\n");
  printf("Ejemplos:\n");
  printf("  ./cifrador --cifrar -i texto.txt -o cifrado.bin -c miClaveSecreta\n");
  printf("  ./cifrador (modo interactivo: pedira los datos por consola)\n\n");
  exit(0);
}


int file_exists(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file != NULL) {
    fclose(file); 
    return 0;   // File exists
  }
  return 1;     
}

/* Parseo de flags por línea de comandos */
void check_flags(int argc, char *argv[], int *modo, char *fichero_inicial, char *fichero_final, unsigned char *clave)
{
  char error_msg[] = {0};
  for (int i = 1; i < argc; i++) 
  {
    if (strcmp(argv[i], "--cifrar") == 0 || strcmp(argv[i], "-c") == 0) 
      *modo = 1;
    else if (strcmp(argv[i], "--descifrar") == 0 || strcmp(argv[i], "-d") == 0) 
      *modo = 2;
    else if ((strcmp(argv[i], "--input") == 0 || strcmp(argv[i], "-i") == 0) && i + 1 < argc) 
    {
    if (argv[i+1][0] == '-') print_help("El nombre del archivo de input no es valido.");
    if (file_exists(argv[i+1]) == 1) print_help("El archivo de input, no existe.");

    strcpy(fichero_inicial, argv[i + 1]);
    i++;
    } 
    else if ((strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0) && i + 1 < argc) 
    {
      // Si el primer caracter de el nombre del archivo entonces asumimos que es una flag 
      if (argv[i+1][0] == '-') print_help("El nombre del archivo de output no es valido.");
      strcpy(fichero_final, argv[i + 1]);
      i++;
    } 
    else if ((strcmp(argv[i], "--clave") == 0 || strcmp(argv[i], "-k") == 0) && i + 1 < argc) 
    {
      strncpy((char *)clave, argv[i + 1], 13);
      clave[13] = '\0';
      i++;
    }
    else if (strcmp(argv[i], "--help") == 0)
    {
      print_help(error_msg);
    }
  }
}

/* Funcion para pedir los datos faltantes interactivamente */
void ask_missing_data(int *modo, char *fichero_inicial, char *fichero_final, unsigned char *clave) 
{
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
    scanf("%s", fichero_inicial);
    clean_buffer();
  }

  if (strlen(fichero_final) == 0) {
    printf("\nEscriba el nombre del fichero de salida: ");
    scanf("%s", fichero_final);
    clean_buffer();
  }

  if (strlen((char *)clave) == 0) {
    printf("\nEscriba la clave (maximo 13 chars): ");
    fgets((char *)clave, 14, stdin);
  }

  printf("\nLa clave es: %s", clave);
  printf("\nLa longitud de la clave es %zu\n", strlen((const char *)clave));
}

/* Este subprograma hace que el registro avance un ciclo completo*/
void avance(void)
{
  short int bb,jj;
  bb = 3 * registro[9] + 2 * registro[2] + 5 * registro[0];
  bb = bb % 251;
  for (jj = 0; jj < 12; jj++) registro[jj] = registro[jj+1];
  registro[12] = bb;
}


int main (int argc, char *argv[])
{
  // He cambiado de unsigned char a char el tipo de las variables nombre de los elementos a los que no se les aplica aritmetica modular
  // Preincializar para que strlen trabaje bien, sino puede dar a error
  char fichero_inicial[300] = {0};
  char fichero_final[300] = {0};

  unsigned char clave[14] = {0};
  unsigned char leocaracter,escribocaracter,cifrante;
  unsigned short int j,k;
  int modo = 0;

  // Parsing de los datos
  check_flags(argc, argv, &modo, fichero_inicial, fichero_final, clave);
  ask_missing_data(&modo, fichero_inicial, fichero_final, clave);

  for(j=0; j<=12; j++)
  {
    if (clave[j]=='\0') 
    // Checkear nullterminated 
    {break;};
    registro[j]=(short)clave[j]% 251;
  }
  k=registro[3];

  for(j=0; j<k; j++)
    avance();

  /*Pasamos a abrir el fichero que queremos cifrar*/

  FILE *fientrada;
  fientrada=fopen(fichero_inicial,"rb");
  if (fientrada==NULL)
  {
    printf("Error: no he podido abrir el fichero de entrada");
  }
  else
{
    /*Abrimos el fichero de salida que estara encriptado*/
    FILE *fisalida;
    fisalida=fopen(fichero_final,"wb");
    if(fisalida==NULL)
    {
      printf("ERROR: no he podido abrir el fichero de salida");
      return 1;
    }
    else
  {
      fread(&leocaracter,sizeof(leocaracter),1,fientrada);

      /*Supongo que leer caracter a caracter es un modo de operacion muy lento, pero...*/

      while(feof(fientrada)==0)
      {
        cifrante= (char) registro[12]; /*pasamos a modo char; es decir en Z/(256) */

        escribocaracter=leocaracter^cifrante; /*suma binaria XOR */

        fwrite(&escribocaracter,sizeof(escribocaracter),1,fisalida);

        avance();

        fread(&leocaracter,sizeof(leocaracter),1,fientrada);
      }
      fclose(fisalida);
    }
    fclose(fientrada);
  }
  clean_buffer();

  printf("\n\nEscriba un caracter y pulse intro para cerrar esta ventana");
  getchar();


  return 0;
}


