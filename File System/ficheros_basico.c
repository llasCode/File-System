//Autores: Lluís Alcover Serra & Román Feria Ibañez
#include "ficheros_basico.h"
#include <limits.h>

#define DEBUG 0 //a 1 para mostrar los mensajes de depuración

// Función encargada de calcular el tamaño en bloques necesario para el mapa de bits
int tamMB(unsigned int nbloques)
{

  // Calculamos el tamaño del mapa de bits
  int tamMB;
  tamMB = (nbloques / 8);

  // Utilizamos el operador módulo para saber si necesitamos esa
  // cantidad justa de bloques o si necesitamos añadir un bloque adicional para los bytes restantes
  if (tamMB % BLOCKSIZE > 0)
  {
    return (tamMB / BLOCKSIZE) + 1;
  }

  return BLOCKSIZE;
}

// Función encargada de calcular el tamaño en bloques del array de inodos
int tamAI(unsigned int ninodos)
{

  int tamAI;
  // Calculamos el tamaño en bloques del array de inodos
  tamAI = (ninodos * INODOSIZE);

  // Utilizamos el operador módulo con BLOCKSIZE para saber si necesitamos
  // esa cantidad justa de bloques o si necesitamos añadir un bloque adicional para el resto, resultado de la división.
  int mod = tamAI % BLOCKSIZE;
  if (mod > 0)
  {
    tamAI = (tamAI / BLOCKSIZE) + 1;
    return tamAI;
  }

  return tamAI / BLOCKSIZE;
}

// Función encargada de inicializar los datos del superbloque
int initSB(unsigned int nbloques, unsigned int ninodos)
{

  struct superbloque SB;

  // Inicializamos todos los campos del superbloque
  SB.posPrimerBloqueMB = posSB + tamSB;
  SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
  SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
  SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
  SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
  SB.posUltimoBloqueDatos = nbloques - 1;
  SB.posInodoRaiz = 0;
  SB.posPrimerInodoLibre = 0;
  SB.cantBloquesLibres = nbloques;
  SB.cantInodosLibres = ninodos;
  SB.totBloques = nbloques;
  SB.totInodos = ninodos;

  // Escribimos el superbloque en la posición posSB
  bwrite(posSB, &SB);

  return 0;
}

// Función encargada de inicializar el mapa de bits poniendo a 1 los bits que representan los metadatos
int initMB()
{

  struct superbloque SB;
  unsigned char buf[BLOCKSIZE];

  memset(buf, 0, BLOCKSIZE);

  // Leemos el superbloque
  if (bread(posSB, &SB) == -1)
  {
    return -1;
  }

  // Escribimos el contenido del buffer en los bloques correspondientes en
  // el mapa de bits, mediante sucesivas llamadas a bwrite
  for (int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++)
  {
    if (bwrite(i, buf) == -1)
    {
      return -1;
    }
  }

  // Escribimos a 1 los bits que representan los bloques ocupados por los metadatos
  for (int i = 0; i <= SB.posUltimoBloqueAI; i++)
  {
    if (escribir_bit(i, 1) == -1)
    {
      return -1;
    }
  }

  // Restamos los bloques ocupados por los metadatos al total de bloques
  SB.cantBloquesLibres -= (SB.posUltimoBloqueAI + 1); // SB + MB + AI

  // Guardamos el superbloque
  if (bwrite(posSB, &SB) == -1)
  {
    return -1;
  }

  return 0;
}

// Función encargada de inicializar la lista de inodos libres
int initAI()
{

  struct superbloque SB;
  // Variable para contabilizar el número de inodos
  int contInodos = 0;
  struct inodo inodos[BLOCKSIZE / INODOSIZE];

  // Leemos el superbloque
  bread(posSB, &SB);

  // Inicializamos el contador de inodos
  contInodos = SB.posPrimerInodoLibre + 1;
  // Recorremos para cada bloque del array de inodos
  for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
  {

    // Recorremos para cada inodo del array de inodos
    for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
    {

      inodos[j].tipo = 'l'; // libre
      // si no hemos llegado al último inodo

      if (contInodos < SB.totInodos)
      {
        // enlazamos con el siguiente
        inodos[j].punterosDirectos[0] = contInodos;
        contInodos++;
      }
      else
      {
        // sinó, hemos llegado al último inodo
        inodos[j].punterosDirectos[0] = UINT_MAX;
        // hay que salir del bucle, el último bloque no tiene por qué estar completo
        break;
      }
    }

    // Escribimos el bloque de inodos en el dispositivo virtual
    bwrite(i, inodos);
  }

  bwrite(0, &SB);

  return 0;
}

// Esta función escribe el valor indicado por el parámetro bit: 0 (libre) ó 1 (ocupado) en
// un determinado bit del MB que representa el bloque nbloque
int escribir_bit(unsigned int nbloque, unsigned int bit)
{

  int posbit;
  int posbyte;
  int nbloqueMB;
  int nbloqueabs;

  unsigned char bufferMB[BLOCKSIZE];
  unsigned char mascara = 128; // 10000000
  struct superbloque SB;

  // Leer el superbloque para obtener la localización del MB
  bread(posSB, &SB);

  // Localizar la posición del byte del bloque físico en el array bufferMB
  posbyte = nbloque / 8;
  posbit = nbloque % 8;
  nbloqueMB = posbyte / BLOCKSIZE;
  nbloqueabs = nbloqueMB + SB.posPrimerBloqueMB;
  bread(nbloqueabs, bufferMB);

  // Ponemos a 1 o 0 los bits correpondientes
  posbyte = posbyte % BLOCKSIZE;

  // desplazamiento de bits a la derecha
  mascara >>= posbit;
  // operador OR para bits
  bufferMB[posbyte] |= mascara;

  // Ponemos el bit a 1
  if (bit == 1)
  {
    bufferMB[posbyte] |= mascara;
  }

  // Ponemos el bit a 0
  else if (bit == 0)
  {
    bufferMB[posbyte] &= ~mascara;
  }
  else
  {
    return -1;
  }

  // Escribimos ese buffer del MB en el dispositivo virtual con bwrite() en la
  // posición que habíamos calculado anteriormente, nbloqueabs
  bwrite(nbloqueabs, bufferMB);

  return 0;
}

// Función encargada de leer un determinado bit del MB y devuelve el valor del bit leído
char leer_bit(unsigned int nbloque)
{

  int posbit;
  int posbyte;
  int nbloqueMB;
  int nbloqueabs = 0;
  unsigned char bufferMB[BLOCKSIZE];
  memset(bufferMB, 0, BLOCKSIZE);

  unsigned char mascara = 128; // 10000000
  struct superbloque SB;

  // Leemos el superbloque
  bread(posSB, &SB);

  // Asignar los valores correspondientes a las posiciones
  // que necesitamos
  posbyte = nbloque / 8;
  posbit = nbloque % 8;
  nbloqueMB = posbyte / BLOCKSIZE;
  nbloqueabs = nbloqueMB + SB.posPrimerBloqueMB;

  bread(nbloqueabs, bufferMB);

  posbyte = posbyte % BLOCKSIZE;

  // desplazamiento de bits a la derecha
  mascara >>= posbit;
  // operador AND para bits
  mascara &= bufferMB[posbyte];
  // desplazamiento de bits a la derecha
  mascara >>= (7 - posbit);

  return mascara;
}

// Función encargada de encontrar el primer bloque libre, consultando el MB (primer bit a 0),
// lo ocupa (poniendo el correspondiente bit a 1 con la ayuda de la función escribir_bit()) y devuelve su posición
int reservar_bloque()
{

  struct superbloque SB;
  unsigned char bufferMB[BLOCKSIZE];
  unsigned char bufferAux[BLOCKSIZE];
  int nbloqueabs;

  int posbyte = 0;
  int bufferByte = 255;

  // llenamos el buffer auxiliar con 1s
  memset(bufferAux, 255, BLOCKSIZE);

  // Leemos el superbloque
  bread(posSB, &SB);

  // Miramos si quedan bloques libres
  if (!SB.cantBloquesLibres)
  {
    return -1;
  }

  nbloqueabs = SB.posPrimerBloqueMB;
  bread(nbloqueabs, bufferMB);

  // Recorremos los bloques MB leyendo y cargando en bufferMB
  while (nbloqueabs <= SB.posUltimoBloqueMB && memcmp(bufferMB, bufferAux, BLOCKSIZE) == 0)
  {
    nbloqueabs++;
    bread(nbloqueabs, bufferMB);
  }

  // localizamos qué byte dentro de ese bloque tiene algún 0
  while (posbyte < BLOCKSIZE && memcmp(&bufferMB[posbyte], &bufferByte, 1) == 0)
  {
    posbyte++;
  }

  // Localizmos el primer bit dentro de ese byte que vale 0
  unsigned char mascara = 128; // 10000000
  int posbit = 0;

  // encontrar el primer bit a 0 en ese byte
  while (bufferMB[posbyte] & mascara)
  { // operador AND para bits
    posbit++;
    // desplazamiento de bits a la izquierda
    bufferMB[posbyte] <<= 1;
  }

  int nbloque;
  // nº de bloque físico que podemos reservar
  nbloque = ((nbloqueabs - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

  // Reservamos
  escribir_bit(nbloque, 1);
  // Decrementamos la cantidad de bloques libres
  SB.cantBloquesLibres--;

  // Salvamos el superbloque
  bwrite(posSB, &SB);

  // Limpiamos bloque
  memset(bufferAux, 0, BLOCKSIZE);

  bwrite(nbloque, bufferAux);
  return nbloque;
}

// Función encargada de liberar un bloque determinado
int liberar_bloque(unsigned int nbloque)
{

  struct superbloque SB;
  // Ponemos a 0 el bit del MB correspondiente al bloque nbloque
  escribir_bit(nbloque, 0);

  // Leemos el superbloque
  bread(posSB, &SB);

  // Decrementamos el número de bloques libres
  SB.cantBloquesLibres++;
  // Salvamos el superbloque
  bwrite(posSB, &SB);

  // Delvovemos el número de bloque liberado
  return nbloque;
}

// Función encargada de escribir el contenido de una variable de tipo struct inodo en un determinado inodo del array de inodos, inodos.
int escribir_inodo(unsigned int ninodo, struct inodo inodo)
{

  struct superbloque SB;
  int nbloqueabs;
  struct inodo inodos[BLOCKSIZE / INODOSIZE];

  // Obtener localización del array de inodos
  bread(posSB, &SB);

  // Calculamos la posición absoluta
  nbloqueabs = (ninodo * INODOSIZE) / BLOCKSIZE + SB.posPrimerBloqueAI;

  // Leemos en su posición absoluta
  bread(nbloqueabs, inodos);

  // Escribimos el inodo en la posición corresposndiente del array
  int pos = ninodo % (BLOCKSIZE / INODOSIZE);
  inodos[pos] = inodo;

  bwrite(nbloqueabs, inodos);

  return 0;
}

// Función encargada de leer un determinado inodo del array de inodos para volcarlo en una variable de tipo struct inodo pasada por referencia
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{

  struct superbloque SB;
  int nbloqueabs;
  struct inodo inodos[BLOCKSIZE / INODOSIZE];

  // Obtenemos la localización del array de inodos
  bread(posSB, &SB);

  // Obtenemos el nº de bloque del array de inodos del inodo solicitado
  nbloqueabs = (ninodo * INODOSIZE) / BLOCKSIZE + SB.posPrimerBloqueAI;

  // Leemos en su posición abosulta
  bread(nbloqueabs, inodos);

  // El inodo solictado esta en pos
  int pos = ninodo % (BLOCKSIZE / INODOSIZE);
  *inodo = inodos[pos];

  return 0;
}

// Función encargada de encontrar el primer inodo libre (dato almacenado en el superbloque),
// reservarlo (con la ayuda de la función escribir_inodo()), devuelve su número y actualiza la lista enlazada de inodos libres
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{

  struct superbloque SB;
  struct inodo inodoAux;

  bread(posSB, &SB);

  // Comprobamos si hay inodos libres
  if (SB.cantInodosLibres <= 0)
  {
    return -1;
  }

  // Guardamos en variable auxiliar cual es el primer inodo libre
  int posInodoReservado;
  posInodoReservado = SB.posPrimerInodoLibre;

  leer_inodo(posInodoReservado, &inodoAux);
  // inicializamos todos los campos del inodo
  SB.posPrimerInodoLibre = inodoAux.punterosDirectos[0];
  inodoAux.tipo = tipo;
  inodoAux.permisos = permisos;
  inodoAux.nlinks = 1;
  inodoAux.tamEnBytesLog = 0;
  inodoAux.atime = time(NULL);
  inodoAux.mtime = time(NULL);
  inodoAux.ctime = time(NULL);
  inodoAux.numBloquesOcupados = 0;

  memset(inodoAux.punterosDirectos, 0, 12 * sizeof(unsigned int));
  memset(inodoAux.punterosIndirectos, 0, 3 * sizeof(unsigned int));

  // Escribir el inodo inicializado en posición del que era el primer inodo libre
  escribir_inodo(posInodoReservado, inodoAux);

  // Decrementamos la cantidad de inodos libres
  SB.cantInodosLibres--;

  // Reescribimos el superbloque
  bwrite(posSB, &SB);
  // Devolvemos la posición del inodo reservado
  return posInodoReservado;
}

// Esta función se encarga de obtener el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar)
{

  struct inodo inodo;
  unsigned int ptr, ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice;
  int buffer[NPUNTEROS];

  leer_inodo(ninodo, &inodo);

  ptr = 0, ptr_ant = 0, salvar_inodo = 0;
  if(DEBUG){
  // printf("traducir_bloque_inodo()→ %d = %d (reservado BF %d para BL %d)]\n", nivel_punteros, ptr, ptr, nblogico);
  }
  // Obtenemos el rango del bloque lógico
  nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
  nivel_punteros = nRangoBL;                           // el nivel_punteros +alto es el que cuelga del inodo

  // iteramos para cada nivel de indirectos
  while (nivel_punteros > 0)
  {
    // Si no cuelgan bloques de punteros
    if (ptr == 0)
    {
      // Error de lectura, bloque inexistente
      if (reservar == 0)
      {
        return -1;
      }
      else
      {
        // reservamos bloques punteros y creamos enlaces desde inodo hasta datos
        salvar_inodo = 1;
        ptr = reservar_bloque(); // de punteros
        
        if (nRangoBL == 0)
        {
          if(DEBUG){
            printf("traducir_bloque_inodo()→ Inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n", nblogico, ptr, ptr, nblogico);
          }
        }else {
          if (nivel_punteros == nRangoBL)
          {
            if (nRangoBL != 0)
            {
              if(DEBUG){
                printf("[traducir_bloque_inodo()-> inodo.punterosIndirectos [%d] = %d (Reservado BF %d para puntero_nivel%d)]\n", nRangoBL - 1, ptr, ptr, nivel_punteros);
              }
            } // Asignamos al último valor de punteros indirectos el bloque reservado

            inodo.punterosIndirectos[nRangoBL - 1] = ptr;
          }
          // En caso contrario
          else
          {
            if(DEBUG){
              printf("[traducir_bloque_inodo()-> punteros_nivel%d [%d] = %d (Reservado BF %d para puntero_nivel%d)]\n", nivel_punteros + 1, indice, ptr, ptr, nivel_punteros);
            }
            // Asignamos el bloque reservado en la posición del buffer corrrespondiente al índice obtenido
            buffer[indice] = ptr;

          }
        }

        
        inodo.numBloquesOcupados++;
        inodo.ctime = time(NULL); // fecha actual

        // Si el bloque cuelga directamente del inodo
        if (nivel_punteros == nRangoBL)
        {
          inodo.punterosIndirectos[nRangoBL - 1] = ptr;

          // sinó, el bloque cuelga de otro bloque de punteros
        }
        else
        {
          buffer[indice] = ptr;
          bwrite(ptr_ant, buffer);
        }
      }
    }

    bread(ptr, buffer);

    indice = obtener_indice(nblogico, nivel_punteros);
    ptr_ant = ptr;        // guardamos el puntero
    ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
    if(DEBUG){
      printf("traducir_bloque_inodo()→ %d = %d (reservado BF %d para BL %d)]\n", nivel_punteros, ptr, ptr, nblogico);
    }
    nivel_punteros--;
  }

  // No existe el bloque de datos
  if (ptr == 0)
  {
    // Error de lectura, no existe el bloque
    if (reservar == 0)
    {
      return -1;
    }
    else
    {

      salvar_inodo = 1;
      ptr = reservar_bloque();
      
      if (nRangoBL == 0)
      {
        if(DEBUG){
          printf("traducir_bloque_inodo()→ Inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n", nblogico, ptr, ptr, nblogico);
        }
      }else {
        if (nivel_punteros == nRangoBL)
        {
          if (nRangoBL != 0)
          {
            if(DEBUG){
              printf("[traducir_bloque_inodo()-> inodo.punterosIndirectos [%d] = %d (Reservado BF %d para puntero_nivel%d)]\n", nRangoBL - 1, ptr, ptr, nivel_punteros);
            }
          } // Asignamos al último valor de punteros indirectos el bloque reservado

          inodo.punterosIndirectos[nRangoBL - 1] = ptr;
        }
        // En caso contrario
        else
        {
          if(DEBUG){
            printf("[traducir_bloque_inodo()-> punteros_nivel%d [%d] = %d (Reservado BF %d para puntero_nivel%d)]\n", nivel_punteros + 1, indice, ptr, ptr, nivel_punteros);
          }
          // Asignamos el bloque reservado en la posición del buffer corrrespondiente al índice obtenido
          buffer[indice] = ptr;
        }
      }

      inodo.numBloquesOcupados++;
      inodo.ctime = time(NULL);

      if (nRangoBL == 0)
      {
        inodo.punterosDirectos[nblogico] = ptr;
      }
      else
      {
        buffer[indice] = ptr;
        bwrite(ptr_ant, buffer);
      }
    }
  }

  if (salvar_inodo == 1)
  {
    // Escribimos el inodo, si lo hemos actualizado
    escribir_inodo(ninodo, inodo);
  }

  return ptr;
}

// Esta función se encarga para obtener el rango de punteros en el que se sitúa el bloque
// lógico que buscamos (0:D, 1:I0, 2:I1, 3:I2), y obtenemos además la dirección almacenada en el puntero correspondiente del inodo, ptr
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{

  if (nblogico < DIRECTOS)
  {

    *ptr = inodo->punterosDirectos[nblogico];
    return 0;
  }
  else if (nblogico < INDIRECTOS0)
  {

    *ptr = inodo->punterosIndirectos[0];
    return 1;
  }
  else if (nblogico < INDIRECTOS1)
  {

    *ptr = inodo->punterosIndirectos[1];
    return 2;
  }
  else if (nblogico < INDIRECTOS2)
  {

    *ptr = inodo->punterosIndirectos[2];
    return 3;
  }
  else
  {

    *ptr = 0;
    perror("Bloque lógico fuera de rango");
    return -1;
  }
}

// Esta función se encarga de la obtención de los índices de los bloques de punteros
int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
  if (nblogico < DIRECTOS)
  {

    return nblogico;
  }
  else if (nblogico < INDIRECTOS0)
  {

    return nblogico - DIRECTOS;
  }
  else if (nblogico < INDIRECTOS1)
  {

    if (nivel_punteros == 2)
    {

      return (nblogico - INDIRECTOS0) / NPUNTEROS;
    }
    else if (nivel_punteros == 1)
    {

      return (nblogico - INDIRECTOS0) % NPUNTEROS;
    }
  }
  else if (nblogico < INDIRECTOS2)
  {

    if (nivel_punteros == 3)
    {

      return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
    }
    else if (nivel_punteros == 2)
    {

      return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
    }
    else if (nivel_punteros == 1)
    {

      return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
    }
  }

  return -1;
}

int liberar_inodo(unsigned int ninodo)
{

  struct superbloque SB;
  struct inodo inodo;

  leer_inodo(ninodo,&inodo);

  int bloquesLiberados = liberar_bloques_inodo(0,&inodo);

  if((inodo.numBloquesOcupados - bloquesLiberados)!=0){
      printf("No se han podido liberar todos los bloques");
      return -1;
  }

  inodo.tipo = 'l';
  inodo.tamEnBytesLog = 0;
  bread(posSB, &SB);

  //Actualizamos el inodo
  int nInodoaux = SB.posPrimerInodoLibre;
  SB.posPrimerInodoLibre = ninodo;
  inodo.punterosDirectos[0] = nInodoaux;
  SB.cantInodosLibres++;
  inodo.numBloquesOcupados-=bloquesLiberados;

  //Escribimos inodo
  escribir_inodo(ninodo, inodo);
  
  bwrite(posSB, &SB);
  
  return ninodo;
}

int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{

  unsigned int nivel_punteros, indice, ptr, nBL, ultimoBL;
  int nRangoBL, ptr_nivel [3], indices[3], liberados;
  unsigned int bloques_punteros [3] [NPUNTEROS];
  unsigned char bufAux_punteros [BLOCKSIZE];
  
  
  //Contadores para contar el número de breads y bwrites
    int cont = 0;
    int cont2 = 0;
  

  liberados = 0;
  if(inodo->tamEnBytesLog == 0) return 0;

  if(inodo->tamEnBytesLog%BLOCKSIZE == 0){
      ultimoBL = inodo->tamEnBytesLog/BLOCKSIZE-1;
  }else{
      ultimoBL=inodo->tamEnBytesLog / BLOCKSIZE;
  }

  memset(bufAux_punteros, 0, BLOCKSIZE);
  ptr = 0;
  if(DEBUG){
    printf("[liberar_bloques_inodo()→ primer BL: %d, último BL: %d]\n",primerBL,ultimoBL);
  }
  for(nBL = primerBL ; nBL <= ultimoBL ; nBL++){ 
    nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); //0:D, 1:I0, 2:I1, 3:I2
    if (nRangoBL < 0) {
        return -1;
    }
    nivel_punteros = nRangoBL; 
      while (ptr > 0 && nivel_punteros > 0) {
          indice = obtener_indice(nBL, nivel_punteros);
          if (indice == 0  || nBL== primerBL){   
              bread(ptr, bloques_punteros[nivel_punteros - 1]);  
              if(DEBUG){
                cont++;  
              }    
          }                
          ptr_nivel[nivel_punteros - 1] = ptr;
          indices[nivel_punteros - 1] = indice;
          ptr = bloques_punteros[nivel_punteros - 1][indice];
          nivel_punteros--; 
      }
      if (ptr > 0) {
          if(DEBUG){
            printf("[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", ptr, nBL); 
          }
          liberar_bloque(ptr);
          liberados++;
          if (nRangoBL == 0){ 
              inodo->punterosDirectos[nBL]= 0;
          }else{
              nivel_punteros = 1;
              while (nivel_punteros <= nRangoBL){
                  indice = indices[nivel_punteros - 1];
                  bloques_punteros[nivel_punteros - 1][indice] = 0;
                  ptr = ptr_nivel [nivel_punteros - 1]; 
                  if(DEBUG){
                    printf("[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n", ptr,nivel_punteros,nBL);          
                  }
                  if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0 ){   
                      liberar_bloque(ptr);
                      liberados++;
                      if (nivel_punteros == nRangoBL) {         
                          inodo->punterosIndirectos[nRangoBL - 1] = 0;
                      }
                      nivel_punteros++;
                  }else{            
                      bwrite(ptr, bloques_punteros[nivel_punteros - 1]);
                      if(DEBUG){
                        cont2++;
                      }
                      nivel_punteros = nRangoBL + 1;
                  }
              }
          }
        }
  }
  if(DEBUG){
    printf("liberar_bloque_inodo()→ TotalBloquesLiberados: %d, totalBreads: %d, totalBwrites: %d]\n", liberados, cont, cont2);
    printf("[liberar_bloques_inodo()→ total bloques liberados: %d]\n",liberados);
  }
  return liberados;
}



