#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>


void muestrameHoraLeds();
void enciendeLeds(int nVecesEnciende, char *cadena);
void codificaBinario();


int main(){

	int salir = 0;
	int opcion;

	while(salir == 0){
		printf("1.- Muestra la hora en los leds\n");
		printf("2.- Codifica en binario (valores 0-7)\n");
		printf("0.- Salir\n");
		printf("Introduce opción (1,2): ");

		scanf("%d", &opcion);
		if(opcion > 0 && opcion < 3 ){
			switch (opcion) {
				case 1:
					muestrameHoraLeds();
					break;
				case 2:
					codificaBinario();
					break;
				default:
					break;
			}
		}
		if(opcion == 0){
			salir = 1;
			printf("Hasta pronto! :)\n");

		}
	}

	return 0;
}


void muestrameHoraLeds(){

	time_t cosa1 = time(0);
	struct tm *cosa2 = localtime(&cosa1);

	int horas = cosa2->tm_hour;
	int minutos = cosa2->tm_min;
	int segundos = cosa2->tm_sec;
	printf("Son las: %d:%d:%d horas\n",horas,minutos,segundos);

	
	//1--> Parpadea el led numlock tantas veces como la hora que sea
	enciendeLeds(horas, "1");
	//2--> Parpadea el led capslock tantas veces como el minuto que sea
	enciendeLeds(minutos, "2");
	//3--> Parpadea el led scrolllock tantas veces como el segundo que sea
	enciendeLeds(segundos, "3");

	//FINALIZACIÓN
	enciendeLeds(5, "123");
}

void enciendeLeds(int nVecesEnciende, char *cadena){
	int filedesc;
	int i;
	filedesc = open("/dev/leds", O_RDWR|O_TRUNC);
    if(filedesc < 0)
        return;

	for (i = 0; i < nVecesEnciende; ++i) {
		if(write(filedesc, cadena, strlen(cadena)*sizeof(char)) != strlen(cadena)*sizeof(char))
			return;

		sleep(1);
		if(write(filedesc, "0", sizeof(char)) != sizeof(char)) //ESCRIBIMOS UN 0 PARA QUE EL LED PARPADEE
			return;

		sleep(1);
	}
	filedesc = close(filedesc);
    if(filedesc < 0)
        return;

}

void codificaBinario(){
	int filedesc;
	int ndecimal;
	char *cadena = "";

	printf("Introduce número a codificar (0-7): ");

	scanf("%d", &ndecimal);
    switch (ndecimal) {
		case 0:
			cadena = "";
			break;
		case 1:
			cadena = "3";
			break;
		case 2:
			cadena = "2";
			break;
		case 3:
			cadena = "23";
			break;
		case 4:
			cadena = "1";
			break;
		case 5:
			cadena = "13";
			break;
		case 6:
			cadena = "12";
			break;
		case 7:
			cadena = "123";
			break;
		default:
			cadena = "123";
			break;
	}

    filedesc = open("/dev/leds", O_RDWR|O_TRUNC);
    if(filedesc < 0)
        return;

	if(write(filedesc, cadena, strlen(cadena)*sizeof(char)) != strlen(cadena)*sizeof(char))
		return;

	filedesc = close(filedesc);
    if(filedesc < 0)
        return;
}
