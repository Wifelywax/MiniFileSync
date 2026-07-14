#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include "../include/registro.h"

//Ubicacion de los arvchivos
#define FIFO_RUTA "/tmp/fifo_backup_so"
#define LOG_RUTA "./backup/historial.log"

extern char ruta_base_absoluta[PATH_MAX];

void crear_logger(void) {
    //Limpiar y Crear nuevo FIFO
    unlink(FIFO_RUTA);
    if (mkfifo(FIFO_RUTA, 0666) == -1) {
        perror("Error al crear el FIFO del logger");
        exit(EXIT_FAILURE);
    }
    //Creacion del Logger
    pid_t pid = fork();

    if (pid == -1) {
        perror("Error al hacer fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        //Abrir conducto para Lectura y Escritura
        int fd_fifo = open(FIFO_RUTA, O_RDWR);
        // Construye la ruta absoluta del archivo de historial.
        char log_ruta[PATH_MAX * 2];
        snprintf(log_ruta, sizeof(log_ruta), "%s/backup/historial.log", ruta_base_absoluta);

         char buffer[512];
        ssize_t bytes_leidos;

        //Abre el archivo de historial
        int fd_log = open(log_ruta, O_WRONLY | O_CREAT | O_APPEND, 0666);
        // Leer continuamente mensajes enviados por otros procesos.
        while ((bytes_leidos = read(fd_fifo, buffer, sizeof(buffer))) > 0) {
            //Guarda en historial
            write(fd_log, buffer, bytes_leidos);
            //Muestra en Consola
          // write(STDOUT_FILENO, buffer, bytes_leidos);
        }
        //Cierre de descriptores y proceso
        close(fd_fifo);
        close(fd_log);
        exit(EXIT_SUCCESS);
    }
}

void registrar_evento(const char *nombre_archivo) {
    int fd_fifo = open(FIFO_RUTA, O_WRONLY);
    if (fd_fifo == -1) return;

   // Obtiene la fecha y hora actual del sistema
    time_t tiempo = time(NULL);
    struct tm *tiempo_local = localtime(&tiempo);
    char mensaje[512];
   
    //Genera un mensaje con el formato:[AAAA-MM-DD HH:MM:SS]
    int longitud = snprintf( mensaje,sizeof(mensaje),
        "[%04d-%02d-%02d %02d:%02d:%02d] copiado/modificado %s\n",
        tiempo_local->tm_year + 1900,
        tiempo_local->tm_mon + 1,
        tiempo_local->tm_mday,
        tiempo_local->tm_hour,
        tiempo_local->tm_min,
        tiempo_local->tm_sec,
        nombre_archivo
    );

    write(fd_fifo, mensaje, longitud);
    close(fd_fifo);
}