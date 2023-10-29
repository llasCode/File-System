//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "ficheros.h"

int main(int argc, char const *argv[])
{
    char nombre_dispositivo[1024];
    unsigned int ninodo;
    unsigned char permisos;

    if (argc != 4)
    {
        perror("Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n ");
        return -1;
    }
    strcpy(nombre_dispositivo, argv[1]);
    //Validación de sintaxis
    if (access(nombre_dispositivo, F_OK) == -1)
    {
        perror("Error: ");
        return -1;
    }
    ninodo = atoi(argv[2]);
    permisos = atoi(argv[3]);
    if (permisos > 7)
    {
        perror("Error: ");
        return -1;
    }
    
    //Montar dispositivo
    bmount(nombre_dispositivo);
    //Llamada a chmod con los argumentos pasados a enteros
    mi_chmod_f(ninodo, permisos);
    //Desmontar dispositivo
    bumount(nombre_dispositivo);

    return 0;
}