//Autores: Román Feria Ibáñez & Lluís Alcover Serra
#include "directorios.h"

int main(int argc, char const *argv[])
{

    if(argc != 4)
    {
        printf("ERROR Sintaxis: ./mi_mkdir <nombre_dispositivo> <permisos> </ruta_directorio/>\n");
        return -1;
    }

    bmount(argv[1]);

    //Comprobar que permisos sea un nº válido (0-7).
    char camino[strlen(argv[3])+2];
    strcpy(camino, argv[3]);
    int permisos_rev = atoi(argv[2]);
    char permisos = '\0';
    if((permisos_rev > 0 && permisos_rev <= 7) || strcmp(argv[2],"0") == 0)
    {
        permisos = *argv[2];
    }
    else if(strcmp(argv[2],"0") != 0)
    {
        fprintf(stderr, "Error: modo inválido: <<9>>\n");
        return -1;
    }

    mi_creat(camino, permisos);

    bumount();

    return 0;
}
