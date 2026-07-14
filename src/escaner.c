#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> //Archivo de cabecera (interaccion con el sistema de archivos)
#include <sys/stat.h>
#include <unistd.h>
#include "../include/escaner.h"

#define MAX_ARCHIVOS 1024
//Almacenar de datos
Metadatos memoria_archivos[MAX_ARCHIVOS];
int total_archivos = 0;

void escanear_directorio(const char *ruta_base){

//Abrir el directorio
 DIR *directorio=opendir(ruta_base);
if(directorio==NULL){
perror("Error al abrir el directorio");
return;
}

struct dirent *entrada;

//Leer el directorio
while((entrada=readdir(directorio))!=NULL){

 //Ignorar los directorios ocultos (evitar bucles)
if(strcmp(entrada->d_name,".")==0 || strcmp(entrada->d_name,"..")==0) 
continue;

//Construirde ruta
char ruta_completa[PATH_MAX];
snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", ruta_base, entrada->d_name);

struct stat info_archivo;

//Leer Metadatos(Verifica si es softlink o un archivo)
if(lstat(ruta_completa, &info_archivo)==-1){
    perror("Error al leer metadatos del archivo");
    continue;
}

//Verificar si se trata de  un directorio
if(S_ISDIR(info_archivo.st_mode)){
    //Recursividad por ser directorio
    escanear_directorio(ruta_completa);
} else{
    //Almacenar metadatos si es archivo o link
    if(total_archivos<MAX_ARCHIVOS){
        //Copiar su ruta
       strncpy(memoria_archivos[total_archivos].ruta, ruta_completa, PATH_MAX - 1);
        //Uso de la estructura Metadatos
        memoria_archivos[total_archivos].num_inodo=info_archivo.st_ino;
        memoria_archivos[total_archivos].tamanio=info_archivo.st_size;
        memoria_archivos[total_archivos].permisos=info_archivo.st_mode;
        memoria_archivos[total_archivos].fecha_modificacion=info_archivo.st_mtime;

        total_archivos++;
    } else{
        //Se trata de guardar mas de 1024
        printf("Limite de archivos alcanzado, no se pueden almacenar mas metadatos\n");
 
    }
  }
} //hola

closedir(directorio);
}



