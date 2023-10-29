//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "ficheros.h"

#define OFFSET1 9000
#define OFFSET2 209000
#define OFFSET3 30725000
#define OFFSET4 409605000
#define OFFSET5 480000000


int main(int argc, char **argv) {

	if(argc != 4){
		printf("Sintaxis: escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
		return -1;
	}
	void *nombre_fichero = argv[1];
	char *string = argv[2];
	int reserva_inodo = atoi(argv[3]);
	int longitud = strlen(string);
	char buffer [longitud];
	int OFFSETS[5] = {9000,209000,30725000,409605000,480000000};
	strcpy(buffer,string);
	char strings[128]; 
	struct STAT stat;

	bmount(nombre_fichero);

	int nInodo = reservar_inodo('f',6);
	printf("longitud texto: %d\n", longitud);
	int bEscritos = 0;
	memset(buffer,0,longitud);
	mi_stat_f(nInodo, &stat);

	struct tm *ts;
	char atime[80];
	char mtime[80];
	char ctime[80];
	ts = localtime(&stat.atime);
	strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&stat.mtime);
	strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
	ts = localtime(&stat.ctime);
	strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);


	for (int i = 0; i < 5; ++i){
		if (reserva_inodo!=0){
			nInodo = reservar_inodo('f',6);
		}
		memset(buffer,0,longitud);
        printf("\nNº inodo reservado: %d\noffset: %d\n", nInodo, OFFSETS[i]);
        bEscritos = mi_write_f(nInodo,string,OFFSETS[i],longitud);
        mi_stat_f(nInodo,&stat);
        printf("Bytes escritos: %d\n", bEscritos);
        printf("Stat.tamEnBytesLog: %d\n", stat.tamEnBytesLog);
        printf("Stat.numBloquesOcupados: %d\n", stat.numBloquesOcupados);
        write(2, strings, strlen(strings));
	}

	bumount();

	return 0;
}



