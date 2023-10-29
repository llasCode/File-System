//Autores: Román Feria Ibáñez & Lluís Alcover Serra

//Mejoras Realizadas:
-Nivel 3 -> init_MB, uso de escribir_bit para desplazar bits
-Nivel 8 -> mi_touch para poder crear ficheros, mejora mi_ls para listar los datos del inodo para cada directorio
-Nivel 9 -> mejora última entrada en el escribir y en el read para mayor optimización
-Nivel 10 -> mi_rmdir.c para diferenciar entre borrar directorio o borrar fichero
-Nivel 13 -> leer entradas antes de entrar en el bucle y guardar en un buffer de entradas

//Valoraciones
-Nivel 13 -> Granularidad de las secciones críticas en lugar de serialziación
-Nivel 13 -> Tiempo de ejecución de la simulación y la verificación menor a un minuto

//Observaciones para el uso
-Directorios.c -> A la hora de ejecutar el script2, poner DEBUG2 a 1 para mostar el número de inodo, en contraposición ponerlo a 0 al ejecutar el script 3