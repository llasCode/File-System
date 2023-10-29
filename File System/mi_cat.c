//Autores: Román Feria Ibáñez & Lluís Alcover Serra
#include "directorios.h"

#define KTAMBUFFER 1500

//Programa que muestra TODO el contenido de un fichero
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "ERROR Sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
        return -1;
    }

    bmount(argv[1]);

    char *camino = argv[2];
    unsigned int offset = 0, bytesLeidos = 0;
    char buf[BLOCKSIZE];
    //Comprobar que la ruta se corresponda a un fichero
    if (camino[strlen(argv[2]) - 1] == '/') {
        fprintf(stderr, "ERROR: La entrada %s no es un fichero\n", camino);
        return -1;
    }
    //Coincidir los bytes leídos con el tamaño en bytes lógico del fichero y con el tamaño físico ç
    //del fichero externo al que redireccionemos la lectura, y se ha de filtrar la basura.
    memset(buf, 0, BLOCKSIZE);
    int errores = mi_read(camino, buf, offset, BLOCKSIZE);
    while (errores > 0) {
        write(1, buf, errores);
        bytesLeidos += errores;
        offset += BLOCKSIZE;
        memset(buf, 0, BLOCKSIZE);
        errores = mi_read(camino, buf, offset, BLOCKSIZE);
    }

    char auxBuf[100];
    sprintf(auxBuf, "\nTotal_leidos %d\n", bytesLeidos);
    write(2, auxBuf, strlen(auxBuf));
    
    bumount();

    return 0;
}