#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../include/escaner.h"

extern Metadatos memoria_archivos[];
extern int total_archivos;

int main(int argc, char *argv[]){
    //Verificar si se ejecuta de forma correcta(./scan <directorio>)
    if(argc != 2){
        fprintf(stderr, "Debes usarlo de la siguiente forma: %s<directorio>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *directorio_origen= argv[1];

    printf("Iniciando el monitor de directorios:\n");
    printf("Escaneando el directorio: %s\n", directorio_origen);

    //Ejecucion del escaner
    escanear_directorio(directorio_origen);

    //Mostrar los resultados con sus parametros
    printf("RESULTADOS\n");
    printf("Total de archivos:%d\n", total_archivos);

    for(int i=0;i<total_archivos;i++){
        printf("[%d] Archivo: %s|Tamanio:%ld| Inodo: %lu\n",
        i+1,
        memoria_archivos[i].ruta,
        (long)memoria_archivos[i].tamanio,
        (unsigned long)memoria_archivos[i].num_inodo);
    }
    printf("========================================");
   






    return EXIT_SUCCESS;
}