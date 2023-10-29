//Autores: Román Feria Ibáñez & Lluís Alcover Serra
#include "directorios.h"

//Programa mi_link.c que crea un enlace a un fichero, llamando a la función mi_link() de la capa de directorios.
int main(int argc, char **argv) {
    
    if (argc != 4) {
        printf("ERROR Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n");
        return -1;
    }
    
    bmount(argv[1]);
    
    const char *camino1 = argv[2], *camino2 = argv[3];

    int errores = mi_link(camino1, camino2);
    if (errores < 0) {
        return -1;
    }
    
    bumount();

    return 0;

}    