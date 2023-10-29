//Autores: Román Feria Ibáñez & Lluís Alcover Serra
#include "verificacion.h"

int main (int argc, char **argv) {

	//Comprobar la sintaxis
	if (argc != 3)
	{
		printf("Sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulacion>");
		return -1;
	}

	//Montar el dispositivo
	bmount(argv[1]);

	//Cálculo num de entradas a partir del stat
	struct STAT stat;
	char *dir = argv[2];
	//Cargar el stat
	mi_stat(dir,&stat);
	if (dir[strlen(dir) - 1] != '/')
	{
		printf("Error: No es un directorio\n");
		return -1;
	}
	//Calcular num de entradas
	int numentradas = stat.tamEnBytesLog / sizeof(struct entrada);

	//Si numentradas != NUMPROCESOS  entonces ERROR 
	if (numentradas != NUMPROCESOS)
	{
		printf("Error: numentradas es diferente al num de procesos \n");
		return -1;
	}

	// Crear el fichero "informe.txt" dentro del directorio de simulación
	char ficInforme[BLOCKSIZE];
	sprintf(ficInforme, "%sinforme.txt", dir);
	mi_creat(ficInforme, 6);
	
	//Leer los directorios correspondientes a los procesos  //Entradas del directorio de simulación
	//Mejora: leer todas de golpe y almacenarlas en un buffer
	struct entrada buffer_entradas[NUMPROCESOS];
	mi_read(dir, buffer_entradas, 0, NUMPROCESOS * sizeof(struct entrada));

	int offset;
	char prueba[BLOCKSIZE];
	struct INFORMACION info;
	for(int i=0; i<NUMPROCESOS;i++)
	{
		info.nEscrituras = 0;
		// Extraer el PID a partir del nombre de la entrada y guardarlo en el registro info
		char *pid = strchr(buffer_entradas[i].nombre, '_');
		if (!pid)
		{
			printf("Error: El pid indicado no existe\n");
			return -1;
		}
		//Guardar en el registro info
		memset(&info,0,sizeof(struct INFORMACION));
		info.pid = atoi(pid + 1);
			
 		//Recorrer secuencialmente el fichero prueba.dat utilizando buffer de N registros de escrituras
		offset = 0;
		memset(prueba, 0, sizeof(prueba));
		sprintf(prueba, "%s%s/prueba.dat", dir, buffer_entradas[i].nombre);
		int cantidadregistros = 256;
		struct REGISTRO buffer_escrituras [cantidadregistros];
		memset(buffer_escrituras, 0, sizeof(buffer_escrituras));	//NUMPROCESOS * sizeof(struct REGISTRO)	
		while(mi_read(prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0){
			for (int j = 0; j < cantidadregistros; j++)
			{
				//Escritura es válida
				if (buffer_escrituras[j].pid == info.pid)
				{
					//Es primera escritura validada
					if (info.nEscrituras == 0)
					{
						info.PrimeraEscritura = buffer_escrituras[j];
						info.UltimaEscritura = buffer_escrituras[j];
						info.MenorPosicion = buffer_escrituras[j];
						info.MayorPosicion = buffer_escrituras[j];
						//ya será la de menor posición puesto que hacemos un barrido secuencial
					}
					else 	//Resto
					{
						//Actualizar si es preciso
						if (buffer_escrituras[j].nEscritura < info.PrimeraEscritura.nEscritura)
						{
							info.PrimeraEscritura = buffer_escrituras[j];
						}
						else if (buffer_escrituras[j].nEscritura > info.UltimaEscritura.nEscritura)
						{
							info.UltimaEscritura = buffer_escrituras[j];
						}
					}
					//Incrementar contador
					info.nEscrituras++;
				}
			}
			offset += sizeof(buffer_escrituras);
			memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
		}
		//Obtener la escritura de la última posición
		mi_stat(prueba, &stat);
		mi_read(prueba, &info.MayorPosicion, stat.tamEnBytesLog - sizeof(struct REGISTRO), sizeof(struct REGISTRO));
	
		char buffer_informe[BLOCKSIZE];
		char fecha[100];
		struct tm *ts;
		//Añadir info al informe.txt por el final
		mi_stat(ficInforme,&stat);
		sprintf(buffer_informe, "PID: %d\n", info.pid);
		sprintf(buffer_informe + strlen(buffer_informe), "Numero de escrituras: %d\n", info.nEscrituras);
	
		//Primera escritura
		ts = localtime(&info.PrimeraEscritura.fecha);
		strftime(fecha, sizeof(fecha), "%a %d-%m-%Y %H:%M:%S", ts);
		sprintf(buffer_informe + strlen(buffer_informe), "Primera escritura\t%u\t%u\t%s\n", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, fecha);
	
		//Última escritura
		ts = localtime(&info.UltimaEscritura.fecha);
		strftime(fecha, sizeof(fecha), "%a %d-%m-%Y %H:%M:%S", ts);
		sprintf(buffer_informe + strlen(buffer_informe), "Última escritura\t%u\t%u\t%s\n", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, fecha);
	
		//Menor posición
		ts = localtime(&info.MenorPosicion.fecha);
		strftime(fecha, sizeof(fecha), "%a %d-%m-%Y %H:%M:%S", ts);
		sprintf(buffer_informe + strlen(buffer_informe), "Menor posición\t\t%u\t%u\t%s\n", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, fecha);
	
		//Mayor posición
		ts = localtime(&info.MayorPosicion.fecha);
		strftime(fecha, sizeof(fecha), "%a %d-%m-%Y %H:%M:%S", ts);
		sprintf(buffer_informe + strlen(buffer_informe), "Mayor posición\t\t%u\t%u\t%s\n", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, fecha);
	
		//Espacios entre escrituras
		sprintf(buffer_informe + strlen(buffer_informe), "\n\n");
			
		//Escribir dentro del fichero
		mi_write(ficInforme, buffer_informe, stat.tamEnBytesLog, strlen(buffer_informe));
		printf("[%d) %d escrituras validadas en %s]\n", i + 1, info.nEscrituras, prueba);
	}
	bumount();
}
