#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include "../include/escaner.h"

#define MAX_ARCHIVOS 1024

Metadatos memoria_archivos[MAX_ARCHIVOS];
int total_archivos = 0;

void escanear_directorio(const char *ruta_base){

    DIR *directorio=opendir(ruta_base);
if(directorio==NULL){
perror("Error al abrir el directorio");
return;
}

struct dirent *entrada;

while((entrada=readdir(directorio))!=NULL){

if(strcmp(entrada->d_name,".")==0 || strcmp(entrada->d_name,"..")==0)
continue;

char ruta_completa[PATH_MAX];

snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", ruta_base, entrada->d_name);

struct stat info_archivo;

if(lstat(ruta_completa, &info_archivo)==-1){
    perror("Error al leer metadatos del archivo");
    continue;
}

if(S_ISDIR(info_archivo.st_mode)){
    escanear_directorio(ruta_completa);
} else{
    if(total_archivos<MAX_ARCHIVOS){

       strncpy(memoria_archivos[total_archivos].ruta, ruta_completa, PATH_MAX - 1);

        memoria_archivos[total_archivos].num_inodo=info_archivo.st_ino;
        memoria_archivos[total_archivos].tamanio=info_archivo.st_size;
        memoria_archivos[total_archivos].permisos=info_archivo.st_mode;
        memoria_archivos[total_archivos].fecha_modificacion=info_archivo.st_mtime;

        total_archivos++;
    } else{
        printf("Limite de archivos alcanzado, no se pueden almacenar mas metadatos\n");
 
    }
  }
}

closedir(directorio);
}



