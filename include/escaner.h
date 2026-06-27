#ifndef ESCANER_H
#define ESCANER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <linux/limits.h> //Para usa PATH_MAX (no valia solo con el limits.h)
#include <limits.h>

typedef struct{
char ruta[PATH_MAX];

/*Partes del inodo(visto en clases)*/

ino_t num_inodo;          // Identificador
off_t tamanio;            // Manejo de archivos grandes 
mode_t permisos;          // Permisos de lectura,escritura, ejecucion

time_t fecha_modificacion;
} Metadatos;

void escanear_directorio(const char* ruta_base);






















#endif