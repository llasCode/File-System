//Autores: Román Feria Ibáñez & Lluís Alcover Serra
#include "directorios.h"

int main(int argc, char **argv) {

	if (argc != 4) {
		printf("ERROR Sintaxis: ./mi_touch <nombre_dispositivo> <permisos> </ruta>\n");
		return -1;
	} 

	bmount(argv[1]);

	unsigned char permisos = *argv[2];
	int auxChar = strlen(argv[3])-1;
	if(argv[3][auxChar]!='/'){
		if(mi_creat(argv[3], permisos) == 0){
			printf("El fichero %s se ha creado correctamente con permisos %c\n", argv[3], permisos);
		} else {
			return -1;
		}
	} 

	bumount();

	return 0;
}
