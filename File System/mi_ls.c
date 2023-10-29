//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "directorios.h"

//Programa (comando) que lista el contenido de un directorio (nombres de las entradas),
//llamando a la función mi_dir() de la capa de directorios, que es quien construye el buffer que mostrará mi_ls.c
int main(int argc, char const *argv[])
{ 
    if(argc != 3)
    {
        fprintf(stderr, "ERROR Sintaxis: ./mi_ls <disco> </ruta_directorio>\n");
        return -1;
    }
    const char *camino = argv[2];
    char buffer[1000000];
    bmount(argv[1]);

    //Llamamos a mi_dir
    mi_dir(camino, buffer);

    fprintf(stderr,"%s\n",buffer);

    bumount();
    
}
