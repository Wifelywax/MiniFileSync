#ifndef ESCANER_H
#define ESCANER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <linux/limits.h>
#include <limits.h>

typedef struct{
char ruta[PATH_MAX];
ino_t num_inodo;
off_t tamanio;
mode_t permisos;
time_t fecha_modificacion;
} Metadatos;

void escanear_directorio(const char* ruta);






















#endif