#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>//open() y banderas
#include <fcntl.h>
#include <libgen.h> //basename() y extraccion datos
#include "../include/workers.h"

#define BUFFER 4096

void procesar_archivo(const char *ruta_origen,int id_worker){

    char cp_ruta[4096];
    strncpy(cp_ruta, ruta_origen, sizeof(cp_ruta));
    char *nombre_archivo = basename(cp_ruta);

    char ruta_destino[4096];
    snprintf(ruta_destino, sizeof(ruta_destino), "./backup/%s",nombre_archivo);


       struct stat stat_origen, stat_destino;

    // 1. Extraemos los metadatos del archivo original
    if (stat(ruta_origen, &stat_origen) == -1) {
        perror("Error al leer metadatos del origen");
        return;
    }

    // 2. Comprobamos si el archivo ya existe en la carpeta backup
    // Si stat() devuelve 0, significa que el archivo destino SÍ existe.
    if (stat(ruta_destino, &stat_destino) == 0) {
        
        // 3. Comparamos el tamaño y la fecha de modificación
        // Si tienen el mismo tamaño Y la fecha del origen no es más reciente que el destino...
        if (stat_origen.st_size == stat_destino.st_size && 
            stat_origen.st_mtime <= stat_destino.st_mtime) {
            
            // ¡No hay cambios! Omitimos la copia para ahorrar recursos.
            printf("[Worker %d] Omitido (sin cambios): %s\n", id_worker, nombre_archivo);
            return; // Terminamos la función aquí mismo
        }
    }

    // ==========================================
    // LÓGICA DE COPIA (Solo llega aquí si es nuevo o cambió)
    // ==========================================
    int fd_origen = open(ruta_origen, O_RDONLY);
    if (fd_origen == -1) {
        perror("Error al abrir el archivo de origen");
        return;
    }

    int fd_destino = open(ruta_destino, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_destino == -1) {
        perror("Error al crear el archivo en backup");
        close(fd_origen);
        return;
    }

    char bufer_memoria[BUFFER];
    ssize_t bytes_leidos, bytes_escritos;

    while ((bytes_leidos = read(fd_origen, bufer_memoria, BUFFER)) > 0) {
        bytes_escritos = write(fd_destino, bufer_memoria, bytes_leidos);
        if (bytes_escritos != bytes_leidos) {
            perror("Error crítico de escritura en el disco destino");
            break;
        }
    }

    if (bytes_leidos == -1) {
        perror("Error durante la lectura del origen");
    } else {
        // Mensaje actualizado para indicar que realmente hizo el trabajo
        printf("[Worker %d] Copiado/Actualizado exitoso: %s\n", id_worker, nombre_archivo);
    }

    close(fd_origen);
    close(fd_destino);
}