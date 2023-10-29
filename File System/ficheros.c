//Autores: Román Feria Ibáñez & Lluís Alcover Serra

#include "ficheros.h"

// Escribe el contenido de un buffer en un fichero/directorio
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{

    struct inodo inodo;
    int primerBL, ultimoBL, desp1, desp2;
    char buf_bloque[BLOCKSIZE];
    int bfisico;

    leer_inodo(ninodo, &inodo);

    if ((inodo.permisos & 2) != 2)
    {
        printf("El inodo no tiene permisos de escritura.\n");
        return -1;
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    mi_waitSem();
    bfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
    mi_signalSem();

    // Primer caso
    if ((primerBL == ultimoBL))
    {
        bread(bfisico, buf_bloque);
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        bwrite(bfisico, buf_bloque);
    }
    else
    { // Segundo caso, caso en que la escritura ocupe mas de un bloque

        // fase 1
        bread(bfisico, buf_bloque);
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        bwrite(bfisico, buf_bloque);

        // fase 2
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            mi_waitSem();
            bfisico = traducir_bloque_inodo(ninodo, i, 1);
            mi_signalSem();
            bwrite(bfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
        }

        // fase 3
        mi_waitSem();
        bfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        mi_signalSem();

        bread(bfisico, buf_bloque);
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
        bwrite(bfisico, buf_bloque);
    }

    mi_waitSem();
    leer_inodo(ninodo, &inodo);
    inodo.mtime = time(NULL);

    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    {
        inodo.ctime = time(NULL);
        inodo.tamEnBytesLog = offset + nbytes;
    }

    escribir_inodo(ninodo, inodo);
    mi_signalSem();
    return nbytes;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    unsigned int primerBLogico, ultimoBLogico, desp1, desp2, leidos, ultimobyte;
    char buf_bloque[BLOCKSIZE];
    unsigned int bfisico;
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == -1)
        return -1;
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, "Error: permiso denegado de lectura\n");
        return -1;
    }
    if (offset >= inodo.tamEnBytesLog)
    {
        leidos = 0;
        return leidos; // No podemos leer nada
    }
    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    {
        nbytes = inodo.tamEnBytesLog - offset; // Leemos hasta el EOF
    }
    primerBLogico = offset / BLOCKSIZE;
    ultimoBLogico = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    ultimobyte = (offset + nbytes - 1);
    if (primerBLogico == ultimoBLogico) // Un solo bloque involucrado
    {
        bfisico = traducir_bloque_inodo(ninodo, primerBLogico, 0);
        if (bfisico != -1)
        {
            if (bread(bfisico, buf_bloque) == -1) {
                return -1;
            }    
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        leidos = nbytes;
    }
    else // Más de un bloque involucrado
    {
        desp2 = (ultimobyte % BLOCKSIZE);
        bfisico = traducir_bloque_inodo(ninodo, primerBLogico, 0);
        if (bfisico != -1)
        {
            if (bread(bfisico, buf_bloque) == -1)
                return -1;
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        leidos = BLOCKSIZE - desp1;
        for (int i = primerBLogico + 1; i < ultimoBLogico; i++)
        {
            bfisico = traducir_bloque_inodo(ninodo, i, 0);
            if (bfisico != -1)
            {
                if (bread(bfisico, buf_bloque) == -1)
                    return -1;
                memcpy(buf_original + leidos, buf_bloque, BLOCKSIZE);
            }
            leidos += BLOCKSIZE;
        }
        bfisico = traducir_bloque_inodo(ninodo, ultimoBLogico, 0);
        if (bfisico != -1)
        {
            if (bread(bfisico, buf_bloque) == -1)
                return -1;
            memcpy(buf_original + leidos, buf_bloque, desp2 + 1);
        }
        leidos += desp2 + 1;
    }

    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        mi_signalSem();
        return -1;
    }
    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, inodo) == -1)
    {
        mi_signalSem();
        return -1;
    }
    mi_signalSem();
    return leidos;
}

// Devuelve la metainformación de un fichero/directorio
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;

    leer_inodo(ninodo, &inodo);
    
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return 0;
}

// Cambia los permisos de un fichero/directorio
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    mi_waitSem();
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);

    inodo.permisos = permisos;
    inodo.ctime = time(NULL); // fecha actual

    escribir_inodo(ninodo, inodo);
    mi_signalSem();
    return 0;
}

// Trunca un fichero/directorio a los bytes indicados como nbytes
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    

    struct inodo inodo;
    unsigned int nblogico;
    leer_inodo(ninodo, &inodo);
    // Miramos si tenemos permisos
    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, "Error en ficheros.c mi_truncar_f() --> Permisos denegados de escritura\n");
        return -1;
    }
    else
    {
        if (inodo.tamEnBytesLog <= nbytes)
        {
            fprintf(stderr, "Error en ficheros.c mi_truncar_f() --> %d: %s\n", errno, strerror(errno));
            return -1;
        }
        // Calculamos nblogico
        if (nbytes % BLOCKSIZE == 0)
        {
            nblogico = nbytes / BLOCKSIZE;
        }
        else
        {
            nblogico = nbytes / BLOCKSIZE + 1;
        }
        int bLiberados = liberar_bloques_inodo(nblogico, &inodo);
        inodo.numBloquesOcupados = inodo.numBloquesOcupados - bLiberados;
        // Actualizamos mtime, ctime y tamEnBytesLog
        inodo.mtime = time(NULL);
        inodo.ctime = time(NULL);
        inodo.tamEnBytesLog = nbytes;
        escribir_inodo(ninodo, inodo);
        return bLiberados;
    }
    fprintf(stderr, "Error en ficheros.c mi_truncar_f() --> %d: %s\n", errno, strerror(errno));
    return -1;
}

