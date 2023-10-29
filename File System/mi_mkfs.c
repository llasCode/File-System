//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "ficheros_basico.h"

//Función que formatea el dispositivo virtual con el tamaño adecuado de bloques
int main(int argc, char **argv) {

    unsigned char buf[BLOCKSIZE];
    memset(buf, 0, BLOCKSIZE);

    int nbloques;
    int ninodos;
    char nombre_disp[BLOCKSIZE];
    nbloques = atoi(argv[2]);

    strcpy(nombre_disp, argv[1]);
    ninodos = nbloques / 4;

    bmount(nombre_disp);

    for (int i = 0; i < nbloques; i++)
    {
        bwrite(i, buf);
    } 
    
    initSB(nbloques, ninodos);
    initMB();
    initAI();

    reservar_inodo('d', 7);

    bumount(); 

    return 0;
}
