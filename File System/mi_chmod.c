//Autores: Román Feria Ibáñez & Lluís Alcover Serra
#include "directorios.h"

int main(int argc, char const *argv[])
{
    if(argc != 4)
    {
        fprintf(stderr,"Sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n");
        return -1;
    }

    bmount(argv[1]);

    const char *camino = argv[3];
    int permisos_rev = atoi(argv[2]);
    char permisos = ' ';
    if(permisos_rev >= 0 && permisos_rev <= 7)
    {
        permisos = argv[2][0];
    }
    else
    {
        fprintf(stderr, "Error: modo inválido: <<9>>\n");
        return -1;
    }

    mi_chmod(camino, permisos);

    bumount();
}