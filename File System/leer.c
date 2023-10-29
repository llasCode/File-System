//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "ficheros.h"

#define KTAMBUFFER 1500

int main(int argc, char **argv)
{   

    //Control de errores (Sintaxis)
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis: leer <nombre_dispositivo> <ninodo>\n");
        return -1;
    }
    
    //Montar dispositivo
    bmount(argv[1]);

    int ninodo = atoi(argv[2]);
    int offset = 0;
    int nbytes = 0;
    int leidos = 0;
    struct STAT stat;
    unsigned char buffer[KTAMBUFFER];
    memset(buffer, 0, KTAMBUFFER);
    
    leidos = mi_read_f(ninodo, buffer, offset, KTAMBUFFER);
    while (leidos > 0)
    {
        write(1, buffer, leidos);
        nbytes += leidos;
        offset += KTAMBUFFER;
        memset(buffer, 0, KTAMBUFFER);
        leidos = mi_read_f(ninodo, buffer, offset, KTAMBUFFER);
    }

    fprintf(stderr, "\nBytes leídos: %d\n", nbytes);
    mi_stat_f(ninodo, &stat);
    fprintf(stderr, "Tamaño en bytes lógicos: %d\n", stat.tamEnBytesLog);

    //Desmontar dispositivo
    bumount();
}



