//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "directorios.h"
#include <sys/wait.h>
#include <signal.h>

#define REGMAX 500000
#define NUMPROCESOS 100 //3
#define NUMESCRITURAS 50 // 10

struct REGISTRO { //sizeof(struct REGISTRO): 24
   time_t fecha; //Precisión segundos
   pid_t pid; //PID del proceso que lo ha creado
   int nEscritura; //Entero con el número de escritura (de 1 a 50)
   int nRegistro; //Entero con el número del registro dentro del fichero
};

void reaper();