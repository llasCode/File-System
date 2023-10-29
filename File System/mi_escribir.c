//Autores: Román Feria Ibáñez & Lluís Alcover Serra
#include "directorios.h"

//Programa que permite escribir texto en una posición de un fichero (offset).
int main(int argc, char **argv) {
	

    if (argc != 5) {
        printf("ERROR Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return -1;
    }

    bmount(argv[1]);

    char *camino = argv[2];
    char *texto = argv[3];
    unsigned int offset = atoi(argv[4]), length = strlen(texto);
    //Miramos que la entradada no sea un directorio
    if (camino[strlen(camino) - 1] == '/') {
        printf("ERROR: La entrada %s no es un fichero\n", camino);
        return -1;
    }
    //Llamamos a mi_write que a su vez llama a mi_write_f (se comprueban los permisos de escritura)
    printf("longitud texto: %zu\n", strlen(texto));
    int errores = mi_write(camino, texto, offset, length);
    if (errores < 0) {
        fprintf(stderr, "ERROR: No se ha podido escribir en la entrada %s\n", camino);
        printf("\nBytes escritos: %d\n", 0);
        return -1;
    }
    printf("\nBytes escritos: %d\n", errores);

    bumount();

    return 0;
}



