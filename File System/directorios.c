//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "directorios.h"

#define DEBUG 0 //a 1 para mostrar los mensajes de depuración
#define DEBUG2 0 //a 1 para mostrar el número de inodo del stat para el script2, QUITAR (poner a 0) para el script3!!

struct UltimaEntrada ultimaEntradaLeida;

//////////////////NIVEL 7//////////////////

// Dado un camino que empieza con '/', separa lo que està contenido entre los primeros dos '/' en inicial y lo demàs en final
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    int i = 0;
    if (camino[0] != '/') {
        return -1;
    }
    const char *sig_camino = strchr(camino + 1,'/');
    memset(inicial,0,strlen(inicial));
    if(sig_camino==NULL){
        strcpy(inicial, camino+1);
        *final = '\0';
        *tipo = 'f'; //Es un file
    } else {
        i = sig_camino - camino - 1;
        strncpy(inicial, camino + 1,i);
        strcpy(final, sig_camino);
        *tipo = 'd'; //Es un directorio
    }
    return 0;
}

void mostrar_error_buscar_entrada(int error) {
   switch (error) {
   case -1: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -2: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -3: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -4: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -5: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -6: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -7: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}

// Esta función una determinada entrada (la parte *inicial del *camino_parcial que nos devuelva extraer_camino())
// entre las entradas del inodo correspondiente a su directorio padre (identificado con *p_inodo_dir)
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, int reservar, unsigned char permisos){
    
    struct superbloque SB;
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
  
    if (bread(posSB, &SB) == -1)
    {
        fprintf(stderr, "Error: no se ha podido leer el bloque\n");
        return -1;
    }
  
    if (strcmp(camino_parcial, "/") == 0) //camino_parcial es “/”    
    {
        *p_inodo = SB.posInodoRaiz; //nuestra raiz siempre estará asociada al inodo 0
        *p_entrada = 0;
        return 0;
    }
  
    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial) + 1);
  
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1){
        return ERROR_CAMINO_INCORRECTO;
    }
    if(DEBUG){
        printf("[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, (int)reservar);
    }
    //buscamos la entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }
  
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada); //cantidad de entradas que contiene el inodo
    num_entrada_inodo = 0; //nº de entrada inicial
    //el buffer de lectura puede ser un struct tipo entrada
    //o mejor un array de las entradas que caben en un bloque, para optimizar la lectura en RAM
    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    if (cant_entradas_inodo > 0) 
    {
        mi_read_f(*p_inodo_dir, &entrada, 0, sizeof(struct entrada));
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(entrada.nombre, inicial) != 0))
        {
            num_entrada_inodo++;
            //previamente volver a inicializar el buffer de lectura con 0s
            memset(entrada.nombre, 0, sizeof(entrada.nombre));
            mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada));
        }
    }
  
    if (strcmp(entrada.nombre, inicial) != 0)  //la entrada no existe
    {
        switch (reservar)
        {
            case 0: //modo consulta. Como no existe retornamos error
                mostrar_error_buscar_entrada(ERROR_NO_EXISTE_ENTRADA_CONSULTA);
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1: //modo escritura 
                    //Creamos la entrada en el directorio referenciado por *p_inodo_dir
                    //si es fichero no permitir escritura
                if (inodo_dir.tipo == 'f')
                {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }
                //si es directorio comprobar que tiene permiso de escritura
                if ((inodo_dir.permisos & 2) != 2)
                {
                    return ERROR_PERMISO_ESCRITURA;
                }
                else
                {
                    strcpy(entrada.nombre, inicial);
                    if (tipo == 'd')
                    {
                        if (strcmp(final, "/") == 0)
                        {
                            entrada.ninodo = reservar_inodo('d', permisos);
                            if(DEBUG){
                                printf("[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, inicial);
                            }
                        }
                        else //cuelgan más diretorios o ficheros
                        {
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    }
                    else //es un fichero
                    {
                        entrada.ninodo = reservar_inodo('f', permisos);
                        if(DEBUG){
                            printf("[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, inicial);
                        }    
                    }
                    if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1)
                    {
                        if (entrada.ninodo != -1) //entrada.inodo != -1
                        {
                            liberar_inodo(entrada.ninodo);
                        }
                        return EXIT_FAILURE; //-1
                    }
                    if(DEBUG){
                        fprintf(stderr, "[buscar_entrada()→ creada entrada: %s, %d]\n", entrada.nombre, entrada.ninodo);
                    }
                }
            }
        }
        if ((strcmp(final, "/") == 0) || strcmp(final, "") == 0)
        {
            if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
            {
                //modo escritura y la entrada ya existe
                mostrar_error_buscar_entrada(ERROR_ENTRADA_YA_EXISTENTE);
                return ERROR_ENTRADA_YA_EXISTENTE;
            }
            // cortamos la recursividad
            *p_inodo = entrada.ninodo;
            *p_entrada = num_entrada_inodo;
            return EXIT_SUCCESS;
        }
        else
        {
          *p_inodo_dir = entrada.ninodo;
            return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
        }
    return EXIT_SUCCESS;
}



//////////////////NIVEL 8//////////////////

// Crea un fichero/directorio y su entrada de directorio
int mi_creat(const char *camino, unsigned char permisos){
    mi_waitSem();   
    unsigned int p_inodo_dir = 0, p_inodo, p_entrada;
    int errores = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    if (errores < 0)
    {
        mostrar_error_buscar_entrada(errores);
        mi_signalSem();
        return -1;
    }
    mi_signalSem();
    return 0;
}

// Función de la capa de directorios que pone el contenido del directorio en un buffer de memoria
int mi_dir(const char *camino, char *buffer){
    unsigned int p_inodo_dir = 0, p_inodo, inicial = 0;
    struct inodo inodo;
    char tmp[100];
    int errores = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &inicial, 0, '4');
    if (errores < 0)
    {
        return -1;
    }
    if (leer_inodo(p_inodo, &inodo) == -1)
    {
        return -1;
    }
    // Comprobamos errores
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, "El fichero '%s' no tiene permisos de lectura.\n", camino);
        return -1;
    }
    if (inodo.tipo != 'd')
    {
        fprintf(stderr, "%s no es un directorio\n", camino);
        return -1;
    }

    //Mejora: mostrar datos del inodo del fichero
    struct inodo inodoEntrada;
    struct tm *tm;
    unsigned int numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

    struct entrada bufferDir[numEntradas];

    errores = leer_inodo(p_inodo, &inodoEntrada);
    if (errores < 0)
    {
        return errores;
    }

    printf("Total: %d\n", numEntradas);
    for (int i = 0; i < numEntradas; i++)
    {
        if (mi_read_f(p_inodo, &bufferDir[i], i * sizeof(struct entrada), sizeof(struct entrada)) < 0)
        {
            fprintf(stderr, "ERROR No se ha podido leer la entrada %d\n", bufferDir[i].ninodo);
            return -1;
        }
        if (leer_inodo(bufferDir[i].ninodo, &inodoEntrada) < 0)
        {
            fprintf(stderr, "ERROR No se ha podido leer el inodo %d\n", bufferDir[i].ninodo);
            return -1;
        }
        char tipo[2], tamEnBytesLog[10];
        tipo[0] = inodoEntrada.tipo;
        tipo[1] = '\0';
        strcat(buffer, tipo);
        strcat(buffer, "\t");

        if (inodoEntrada.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
        if (inodoEntrada.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
        if (inodoEntrada.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");

        strcat(buffer, "\t");
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");
        sprintf(tamEnBytesLog, "%u", inodoEntrada.tamEnBytesLog);
        strcat(buffer, tamEnBytesLog);
        strcat(buffer, "\t");
        strcat(buffer, bufferDir[i].nombre);
        strcat(buffer, "\n");
    }
    return 0;
}

// Función que permite cambiar los permisos de un fichero o de un directorio
int mi_chmod(const char *camino, unsigned char permisos){
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int errores = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if (errores < 0)
    {
      mostrar_error_buscar_entrada(errores);
      return errores;
    }
    if (mi_chmod_f(p_inodo, permisos) < 0)
    {
        return -1;
    }
    return 0;
}

// Función que buscar la entrada *camino con buscar_entrada() para obtener el p_inodo.
// Si la entrada existe llamamos a la función correspondiente de ficheros.c pasándole el p_inodo
int mi_stat(const char *camino, struct STAT *p_stat){
    unsigned int ninodo = 0, p_inodo, inicial;
    //Buscamos la entrada
    int errores = buscar_entrada(camino, &ninodo, &p_inodo, &inicial, 0, '4');
    if (errores < 0) {
        mostrar_error_buscar_entrada(errores);
        return errores;
    }
    if(DEBUG2){
        printf("Nº de inodo: %d\n", p_inodo);
    }
    //La entrada existe, llamamos a mi_stat_f
    errores = mi_stat_f(p_inodo, p_stat);
    if (errores < 0) {
        fprintf(stderr, "ERROR No se ha podido obtener la información del inodo %d\n", ninodo);
        return errores;
    }
    return 0;
}

//////////////////NIVEL 9/////////////////
//Función de directorios.c para leer los nbytes del fichero indicado por camino
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){

    unsigned int p_inodo_dir = 0, p_inodo, p_entrada;
    if(strcmp (camino, ultimaEntradaLeida.camino) == 0) 
    {
		p_inodo = ultimaEntradaLeida.p_inodo;
    } 
    else {
		if(buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, '4') < 0) {
            return -1;
        }
		strcpy(ultimaEntradaLeida.camino, camino);
		ultimaEntradaLeida.p_inodo = p_inodo;
    }
    int bytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes);
	return bytesLeidos;
}

//Función de directorios.c para escribir contenido en un fichero
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
	unsigned int p_inodo_dir = 0, p_inodo, p_entrada;
    if(strcmp(camino, ultimaEntradaLeida.camino) == 0) 
    {
		p_inodo = ultimaEntradaLeida.p_inodo;
    } 
    else {
        //Buscamos la entrada para obtener el p_inodo
	    if(buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, '6') < 0) {
            return -1;
        }
		strcpy(ultimaEntradaLeida.camino, camino);
		ultimaEntradaLeida.p_inodo = p_inodo;
	}
    //La entrada existe, llamamos a mi_write pasandole p_inodo
	int bytesEscritos = mi_write_f(p_inodo, buf, offset, nbytes);
	if(bytesEscritos < 0) {
        return -1;
    }
	return bytesEscritos;
}

//////////////////NIVEL 10/////////////////
//Crea el enlace de una entrada de directorio camino2 al inodo especificado por otra entrada de directorio camino1 .
int mi_link(const char *camino1, const char *camino2){
    mi_waitSem();
	unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
	struct entrada entrada;
    struct inodo inodo;
    
    //Hay que comprobar que la entrada camino1 exista
	if(buscar_entrada(camino1,&p_inodo_dir,&p_inodo,&p_entrada,0,'0') < 0) {
        mi_signalSem();
        return -1;
    }
  
    int ninodo = p_inodo;
	if(leer_inodo(ninodo, &inodo) == -1) {
        mi_signalSem();
        return -1;
    }
    
    //Comprobamos que camino1 tiene permiso de lectura
    if(inodo.tipo != 'f' && (inodo.permisos & 4) != 4) {
        mi_signalSem();
        return -1;
    }
    p_inodo_dir = 0;
    //La entrada de camino2 no tiene que existir, la creamos mediante la función buscar_entrada() con permisos 6
	if(buscar_entrada(camino2,&p_inodo_dir,&p_inodo,&p_entrada,1,'6') < 0) {
        mi_signalSem();
        return -1;
    }
  
    //Leemos la entrada creada correspondiente a camino2, o sea la entrada p_entrada2 de p_inodo_dir2.
	if(mi_read_f(p_inodo_dir,&entrada,p_entrada*sizeof(struct entrada),sizeof(struct entrada))==-1) {
        mi_signalSem();
        return -1;
    }
    
    //Liberamos el inodo que se ha asociado a la entrada creada, p_inodo2. 
	liberar_inodo(entrada.ninodo);

    //Creamos el enlace: Asociamos a esta entrada el mismo inodo que el asociado a la entrada de camino1, es decir p_inodo1.
	entrada.ninodo = ninodo;
   
    //Escribimos la entrada modificada en p_inodo_dir2.
	if(mi_write_f(p_inodo_dir,&entrada,p_entrada*sizeof(struct entrada),sizeof(struct entrada))==-1) {
        mi_signalSem();
        return -1;
    }
    
	if(leer_inodo(ninodo, &inodo) == -1) {
        mi_signalSem();
        return -1;
    }
    //Incrementamos la cantidad de enlaces (nlinks) de p_inodo1, actualizamos el ctime y lo salvamos.
	inodo.nlinks++;
	inodo.ctime = time(NULL);
    escribir_inodo(ninodo, inodo);
    mi_signalSem();
    return 0;

}

//Función de la capa de directorios que borra la entrada de directorio especificada
int mi_unlink(const char *camino){
    mi_waitSem();

    //Variables
	unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    
    //Comprobamos si el camino existe
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error < 0){
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return -1;
    }

    // Si existe la entrada, obtenemos su numero leyendo el inodo 
    struct inodo inodo_entrada;
    
    if (leer_inodo(p_inodo, &inodo_entrada) == -1){
        printf("Error: No se ha podido leer p_inodo\n");
        mi_signalSem();
        return -1;
    }
    //Leemos el inodo que esta asociado al directorio
    struct inodo inodo_dir;

    if (leer_inodo(p_inodo_dir, &inodo_dir) == -1){
        printf("Error: No se ha podido leer p_inodo_dir\n");
        mi_signalSem();
        return -1;
    }

    // btenemos el numero de entradas asociado al inodo
    int numentradas = inodo_dir.tamEnBytesLog / sizeof (struct entrada);

    //Si es un directorio no vacio, salimos ya que este no se puede borrar
    if (inodo_entrada.tamEnBytesLog != 0 && inodo_entrada.tipo == 'd'){
        printf("Error: El directorio %s no está vacío\n",camino);
        mi_signalSem();
        return -1;
    }

    // Si la entrada a eliminar es la ultima truncamos el inodo con su tamaño menos el tamaño de una entrada
    if (p_entrada != numentradas - 1){
        struct entrada entrada;
        //Leemos la ultima entrada
        if (mi_read_f (p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog - sizeof(struct entrada), sizeof(struct entrada)) == -1)
        {
            printf("Error al leer la última entrada\n");
            mi_signalSem();
            return -1;
        }
        // Colocamos en la posicion de entrada lo que queremos eliminar
        if (mi_write_f (p_inodo_dir, &entrada, p_entrada * 
        sizeof(struct entrada), sizeof(struct entrada)) == -1)
        {
            printf("Error: No se ha podido ejecutar mi_write_f() correctamente\n");
            mi_signalSem();
            return - 1;
        }
    }
    // Truncamos el inodo a su tamaño menos el tamaño de una entrada
    mi_truncar_f (p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof (struct entrada)); 
    
    if (leer_inodo(p_inodo, &inodo_dir) == -1){
        printf("Error al leer inodo en mi_unlink\n");
        mi_signalSem();
        return -1;
    }
    // Decrementamos el numero de enlaces
    inodo_dir.nlinks--; 
    //printf("NºLinks: %d\n", inodo_dir.nlinks);
    // Si era el último enlace lo borramos  
    if (inodo_dir.nlinks == 0) {
        liberar_inodo (p_inodo);
        //printf("Liberados: %d\n", x+1);
    }
    else{
        //Si no se borra, actualizamos el inodo
        inodo_dir.ctime = time (NULL);

        if (escribir_inodo (p_inodo, inodo_dir) == -1)
        {
            printf("Error al salvar inodo después de desenlazar\n"); 
            mi_signalSem();
            return -1;
        }
        //printf("Liberados: %d\n", x);
    }
    mi_signalSem();
    return 0;

}

