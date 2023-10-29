//Autores: Román Feria Ibáñez & Lluís Alcover Serra
#include "directorios.h"

#define DEBUG 0 //a 1 para mostrar los mensajes de depuración

void mostrar_buscar_entrada(char *camino, char reservar){
  unsigned int p_inodo_dir = 0;
  unsigned int p_inodo = 0;
  unsigned int p_entrada = 0;
  int error;

  printf("\ncamino: %s, reservar: %d\n", camino, reservar);
  if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
    mostrar_error_buscar_entrada(error);
  }

  printf("**********************************************************************\n");
  return;
}

int main(int argc, char **argv) {

    struct superbloque SB;

    if (argv[1] == NULL) {
        fprintf(stderr,"Sintaxis: leer_sf <nombre_dispositivo>\n");
        return -1;
    }

    //Montamos el disco
    bmount(argv[1]);
    //Leemos el SuperBloque
    bread(posSB, &SB);

    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB=%d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB=%d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI=%d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI=%d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos=%d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos=%d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz=%d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre=%d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres=%d\n", SB.cantBloquesLibres);
    printf("cantInodosLibres=%d\n", SB.cantInodosLibres);
    printf("totBloques=%d\n", SB.totBloques);
    printf("totInodos=%d\n", SB.totInodos);

    if(DEBUG){
        printf("sizeof struct superbloque: %lu\n", sizeof(struct superbloque));
        printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));

        printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES:\n");
        struct inodo inodo[BLOCKSIZE/INODOSIZE];

        for (size_t i=SB.posPrimerBloqueAI; i<=SB.posUltimoBloqueAI; i++) {
            for (size_t j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
                (bread(i, inodo));

                if (i >= SB.posPrimerBloqueAI && i < SB.posPrimerBloqueAI+3) {
                    printf("%d ",inodo[j].punterosDirectos[0]);
                }
                if (i >= SB.posPrimerBloqueAI && i < SB.posUltimoBloqueAI-3) {
                    if (i % 500 == 0) {
                        printf(". ");
                    }
                }
                if (i >= SB.posUltimoBloqueAI-3 && i <= SB.posUltimoBloqueAI) {
                    printf("%d ",inodo[j].punterosDirectos[0]);
                }
            }
        }

        printf("\n");
    
        //Mostrar creación directorios y errores
        mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
        mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
        mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
        mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
        mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
        mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
        mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  
        //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
        mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
        mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
        mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
        mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
        mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
    }

    bumount();

    return 0;
}
