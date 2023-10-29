//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "bloques.h"
#include "semaforo_mutex_posix.h"

//Descriptor de fichero
static int descriptor = 0;

static sem_t *mutex;
static unsigned int inside_sc = 0;


// Funciones de semáforos
void mi_waitSem() {
    if (!inside_sc) waitSem(mutex);
    inside_sc++;
}

void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) signalSem(mutex);
}


//bmount, función encargada de montar el dispositivo virtual, abrirlo
int bmount(const char *camino) {

  if (descriptor > 0) {
      close(descriptor);
  }

  umask(000);
   
  //Obtenermos el despcriptor de fichero
  if ((descriptor = open(camino, O_RDWR | O_CREAT, 0666)) == -1)
  {
    perror("Error: ");
  }

  if (!mutex)
  { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
    mutex = initSem();
    if (mutex == SEM_FAILED)
    {
        return -1;
    }
  }

  //Devolvemos el descriptor de fichero
  return descriptor;
    
}


//bumont, función encargada de desmontar el dispositivo virtual, liberar el descriptor de ficheros
int bumount() {

   //Liberamos el descriptor de fichero
   descriptor = close(descriptor);

   if(descriptor == -1){
      perror("Error: ");
      return -1;
   }

   deleteSem(); 
   return 0;

}



//bwrite, función encargada de escribir 1 bloque en el dispositivo virtual, en el bloque físico especificado por nbloque
int bwrite(unsigned int nbloque, const void *buf) {

   //Calculamos el desplazamiento donde hay que escribir
   int des = (nbloque * BLOCKSIZE);
   int inicio = SEEK_SET;


   int bytes;
   //Movemos el puntero del fichero en el offset correcto
   if (lseek(descriptor, des, inicio) < 0)
   {
      perror("Error: ");
      return -1;
   }

   //Escribimos y devolvemos el número de bytes que se han podido escribir
   if ((bytes = write(descriptor, buf, BLOCKSIZE)) < 0)
   {
      perror("Error: ");
      return -1;
   }

   return bytes;

}


//bread, función encargada de leer 1 bloque del dispositivo virtual, que se corresponde con el bloque físico especificado por nbloque.
int bread(unsigned int nbloque, void *buf) {

   //Calculamos el desplazamiento donde hay que leer
   int des = (nbloque * BLOCKSIZE);
   int inicio = SEEK_SET;

   int bytes;
   //Movemos el puntero del fichero en el offset correcto
   if (lseek(descriptor, des, inicio) < 0)
   {
      perror("Error: ");
      return -1;
   }

   //Leemos y devolvemos el número de bytes que se han podido leer
   if ((bytes = read(descriptor, buf, BLOCKSIZE)) < 0)
   {
      perror("Error: ");
      return -1;
   }

   return bytes;


}


    
