/* Programa de cifrajo en flujo basico y sencillo;
un intento pedagogico y para recordar algo la programacion en C*/

#include <stdio.h>
#include <string.h>


/*Variable global que representa el vector de estado de la sucesion cifrante*/
/*Es menos seguro el uso de variables globales, pero mas sencillo*/
static short int registro[13] = {0,0,0,0,0,0,0,0,1,2,3,4,5};



/* Este subprograma hace que el registro avance un ciclo completo*/
void avance(void)
{
short int bb,jj;
	bb=3*registro[9]+2*registro[2]+5*registro[0];
	bb= bb % 251;
	for (jj=0; jj<12 ;jj++)
		registro[jj]=registro[jj+1];
	registro[12]=bb;
}


int main (void)
{
	/*declaramos variables*/
	unsigned char fichero_inicial[300];
	unsigned char fichero_final[300];
	unsigned char clave[14];
	unsigned char leocaracter,escribocaracter,cifrante;
	unsigned short int j,k,hh;

	/* Leemos el nombre del fichero a cifrar*/
	/* Podemos recordar que no se escriban espacios en blanco*/

	printf("\nEscriba el nombre del fichero que quiere cifrar o descifrar\n");
	printf("O bien arrastre el fichero hasta esta ventana y sueltelo");
	printf("Recuerde que para escribir \\ debe escribir \\\\\n");
	scanf("%s", fichero_inicial);
	while ((hh = getchar()) != '\n' && hh != EOF);

	printf("\nEscriba el nombre del fichero resultante\n");
	printf("Recuerde que para escribir \\ debe escribir \\\\\n");
	scanf("%s", fichero_final);
	while ((hh = getchar()) != '\n' && hh != EOF);
	/*Leemos la clave */

	printf("\nLA CLAVE: debe tener un maximo de 13 caracteres o numeros\nAPUNTALA EN UN PAPEL antes de escribirla\n");
	printf("maximo 13 caracteres.\nEscribe la clave:");
	printf("Si la clave tiene menos de 13 caracteres,\n se completara con el caracter (salto de linea=10)\n");
	printf("y con caracteres nulos=0.\n\n");
	/*scanf("%13s",clave);*/
	fgets(clave,14,stdin);

	while ((hh = getchar()) != '\n' && hh != EOF);
	printf("\nLa clave es: %s\n", clave);
	printf("\n-------------------------\n");
	printf("La longitud de la clave es %d\n",strlen(clave));


	for(j=0; j<=12; j++)
		{
		if (clave[j]==10) /* el salto de linea es el caracter numero 10 */
			{break;};
			registro[j]=(short)clave[j]% 251;
		}
	printf("\n Combrobacion de lo que esta almacenado en el registro\n");
	for(j=0; j<=12; j++)
		{
		printf("%d\n",registro[j]);
		}/*Estas ultimas cinco lineas se pueden quitar*/

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
	while ((hh = getchar()) != '\n' && hh != EOF);
	printf("\n\nEscriba un caracter y pulse intro para cerrar esta ventana");
	getchar();


}
