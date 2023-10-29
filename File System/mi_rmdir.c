//Autores: Román Feria Ibáñez & Lluís Alcover Serra
#include "directorios.h"

//Programa mi_rdmir.c que borra un directorio, llamando a la función mi_unlink() de la capa de directorios.
int main(int argc, char **argv){
    //Comprobamos sintaxis
	if(argc != 3){
		printf("ERROR Sintaxis: ./mi_rmdir <disco> </ruta>\n");
		return -1;
    }
    
	// Miramos si es un fichero o directorio (solo debe desenlazar un directorio)
    const char *camino = argv[2];
    if(camino[strlen(camino)-1] != '/') { 
        fprintf(stderr, "Error, no es un directorio");
        return -1;
    }

    bmount(argv[1]);

    //Llamamos a mi_unlink
    mi_unlink(argv[2]);
    
	bumount();

    return 0;
}