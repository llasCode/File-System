//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "ficheros.h"


int main(int argc, char **argv)
{
  if (argc != 4)
  {
    printf("Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
    return -1;
  }
  else
  {
    //Montamos el disco
    bmount(argv[1]);

    //Obtenemos los parámetros del usuario
    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);

    //Si los bytes a truncar son 0 liberamos el inodo
    if (nbytes == 0)
    {
      liberar_inodo(ninodo);
    }
    else
    {
      mi_truncar_f(ninodo, nbytes);
    }

    //Mostramos la información inodo
    struct STAT info_inodo;
    mi_stat_f(ninodo, &info_inodo);

    printf("INFORMACIÓN DEL INODO %d\n", ninodo);
    printf("----------------------\n");
    printf("tipo: %c\n", info_inodo.tipo);
    printf("permisos: %d\n", info_inodo.permisos);
    char mitime[128];
    strftime(mitime, 128, "%d/%m/%y %H:%M:%S", localtime(&info_inodo.atime));
    printf("atime: %s\n", mitime);
    strftime(mitime, 128, "%d/%m/%y %H:%M:%S", localtime(&info_inodo.ctime));
    printf("ctime: %s\n", mitime);
    strftime(mitime, 128, "%d/%m/%y %H:%M:%S", localtime(&info_inodo.mtime));
    printf("mtime: %s\n", mitime);
    printf("nlinks: %d\n", info_inodo.nlinks);
    printf("tamEnBytesLog: %d\n", info_inodo.tamEnBytesLog);
    printf("numBloquesOcupados : %d\n", info_inodo.numBloquesOcupados);

    //Desmontamos el disco
    bumount();
  }
  return 0;
}

