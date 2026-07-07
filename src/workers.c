#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <libgen.h> //basename() y extraccion datos
#include "../include/workers.h"
#include "../include/utilidades_ipc.h"
#include "../include/registro.h"

#define BUFFER 4096
extern char ruta_base_absoluta[PATH_MAX];

void procesar_archivo(const char *ruta_origen,int id_worker){
    //Copia ruta de archivo
    char cp_ruta[4096];
    strncpy(cp_ruta, ruta_origen, sizeof(cp_ruta));
    //Obtener nombre del archivo
    char *nombre_archivo = basename(cp_ruta);

    char ruta_destino[PATH_MAX * 2];
   snprintf(ruta_destino, sizeof(ruta_destino), "%s/backup/%s", ruta_base_absoluta, nombre_archivo);


       struct stat stat_origen, stat_destino;

    // Extraer los metadatos del archivo original
    if (stat(ruta_origen, &stat_origen) == -1) {
        perror("Error al leer metadatos del origen");
        return;
    }

    //Comprobar si el archivo ya existe en la carpeta backup
    if (stat(ruta_destino, &stat_destino) == 0) {
        
        //Ver si es necesario volver a copiarlo
        if (stat_origen.st_size == stat_destino.st_size && 
            stat_origen.st_mtime <= stat_destino.st_mtime) {
            
          
            printf("[Worker %d] Omitido (sin cambios): %s\n", id_worker, nombre_archivo);
            return;
        }
    }

    //Ver el archivo original (Lectura)
    int fd_origen = open(ruta_origen, O_RDONLY);
    if (fd_origen == -1) {
        perror("Error al abrir el archivo de origen");
        return;
    }
    // Crea o reemplaza el archivo dentro del backup
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
    //Bloquea el semaforo
    sem_wait(estadisticas_semaforos); 
    
    estadisticas_globales->archivos_copiados++;
    //Sumar tamaño del archivo copiado al total de bytes respaldados
    estadisticas_globales->bytes_copiados += stat_origen.st_size;

    registrar_evento(nombre_archivo);
    //Libera el semaforo
    sem_post(estadisticas_semaforos); 

    close(fd_origen);
    close(fd_destino);

    //Error del WSL
    struct utimbuf tiempos;
    tiempos.modtime = stat_origen.st_mtime; // Copia tiempo de modificación exacto
    
    // Sobrescribe los metadatos del backup para que sean idénticos al original
    if (utime(ruta_destino, &tiempos) == -1) {
        perror("Error al clonar los metadatos de tiempo");
    }
}