//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "simulacion.h"

#define DEBUG 0 //a 1 para mostrar los mensajes de depuración

//Contador procesos finalizados
int acabados =0;

//Función enterrador
void reaper(){
  pid_t ended;
  signal(SIGCHLD, reaper);
  while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
    acabados++;
  }
}

int main(int argc, char **argv) {

    //Asociamos la señal SIGCHILD al reaper
    signal(SIGCHLD, reaper);

    //Comprobamos la sintaxis
    if (argc != 2){
        printf("Sintaxis: ./simulacion <disco>");
        return -1;
    }

    //Montamos el dispositivo
    if(bmount(argv[1]) == -1){
        printf("Error al montar el disco\n");
        return -1;
    }

    //Creamos el directorio de simulación con el formato:  /simul_aaaammddhhmmss/
    char *dir1=(char *) malloc(sizeof(char)*60);
    char *tiempoActual = (char *)malloc(sizeof(char)*16);      
    
    time_t fecha = time(NULL);
    struct tm *t;
    int error;

    t=localtime(&fecha);
    sprintf(tiempoActual,"/simul_%04d%02d%02d%02d%02d%02d/", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    strcat(dir1,tiempoActual);
    if((error = mi_creat(dir1,7))< 0){
        printf("Error al crear el directorio");
        mostrar_error_buscar_entrada(error);
        bumount();
        return -1;
    }

    printf("*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS *** \n", NUMPROCESOS, NUMESCRITURAS);
    pid_t pid;
    char *dir2 = (char *) malloc(sizeof(char)*16);

    for (int proceso = 1; proceso <= NUMPROCESOS; proceso++) {
        pid = fork();
        //Si pid == 0, es el hijo
        if(pid == 0){

            //Montamos el dispositivo
            bmount(argv[1]);

            //Creamos el directorio del proceso 
            sprintf(dir2, "proceso_%d/", getpid());
            strcat(dir1, dir2);
            if((error = mi_creat(dir1,7))< 0){
                printf("Error al crear el directorio del proceso ");
                mostrar_error_buscar_entrada(error);
                bumount();
                return -1;
            }

            //Creamos el fichero de texto prueba.dat
            strcat(dir1, "prueba.dat");
            if((error = mi_creat(dir1,7) < 0)){
                printf("Error al crear el fichero prueba.dat");
                mostrar_error_buscar_entrada(error);
                bumount();
                return -1;
            }

            //Semilla de números aleatorios
            srand(time(NULL) + getpid());
            struct REGISTRO registro;

            for(int nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++){
                //Asignamos la información al registro
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro =  rand() % REGMAX;
                //Escribir el registro con mi_write()
                if((error = mi_write(dir1,&registro,registro.nRegistro * sizeof(struct REGISTRO),sizeof(struct REGISTRO))) < 0){
                    printf("Error al escribir el registro %s",dir1);
                    mostrar_error_buscar_entrada(error);
                    bumount();
                    return -1;
                }
                if(DEBUG){
                    printf("[simulación.c -> Escritura %d en %s]\n", nescritura, dir1);
                }
                //Esperamos 0,05 segundos para la siguiente escritura
                usleep(50000);
            }

            //Desmontamos el dispositivo
            bumount();
  
            printf("[Proceso %d: Completadas %d escrituras en %s]\n", proceso, NUMESCRITURAS, dir1);
            exit(0);
        }
        //Esperamos 0,15 segundos para lanzar siguiente proceso
        usleep(150000);
    }

    //Hacemos que el proceso padre espera para los hijos
    while(acabados < NUMPROCESOS){
        pause();
    }

    //Desmontamos el dispositivo
    if (bumount() == -1) {
        fprintf(stderr, "Error en al desmontar el disco\n");
        return -1;
    }

    exit(0);

}