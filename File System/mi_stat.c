//Autores: Román Feria Ibáñez & Lluís Alcover Serra
#include "directorios.h"

//Programa (comando) que muestra la información acerca del inodo de un fichero o directorio,
//llamando a la función mi_stat() de la capa de directorios, que a su vez llamará a mi_stat_f() de la capa de ficheros
int main(int argc, char const *argv[])
{
	if (argc != 3) {
        puts("ERROR Sintaxis: ./mi_stat <disco> </ruta>");
        return -1;
    }

    bmount(argv[1]);

    const char *camino = argv[2];
    struct STAT stat;
	//Llamamos a mi_stat
    int errores = mi_stat(camino, &stat);
    if (errores < 0) {
        fprintf(stderr, "mi_stat.c --> Error al obtener al información de %s\n", camino);
        return errores;
    }

	//Imprimimos toda la información
    printf("Tipo: %c\n",stat.tipo);
    printf("Permisos: %c\n",stat.permisos);
    printf("atime: %s",ctime(&stat.atime));
    printf("mtime: %s",ctime(&stat.mtime));
    printf("ctime: %s",ctime(&stat.ctime));
    printf("nlinks: %u\n",stat.nlinks);
    printf("tamEnBytesLog: %u\n",stat.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n",stat.numBloquesOcupados);

    bumount();
    
    return errores;
}