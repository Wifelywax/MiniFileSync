#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include "../include/registro.h"

#define FIFO_RUTA "/tmp/fifo_backup_so"
#define LOG_RUTA "./backup/historial.log"

void crear_logger(void) {
    unlink(FIFO_RUTA);
    if (mkfifo(FIFO_RUTA, 0666) == -1) {
        perror("Error al crear el FIFO del logger");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("Error al hacer fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        int fd_fifo = open(FIFO_RUTA, O_RDONLY);
        int fd_log = open(LOG_RUTA, O_WRONLY | O_CREAT | O_APPEND, 0666);

        char buffer[512];
        ssize_t bytes_leidos;

        while ((bytes_leidos = read(fd_fifo, buffer, sizeof(buffer))) > 0) {
            write(fd_log, buffer, bytes_leidos);
            write(STDOUT_FILENO, buffer, bytes_leidos);
        }

        close(fd_fifo);
        close(fd_log);
        exit(EXIT_SUCCESS);
    }
}

void registrar_evento(const char *nombre_archivo) {
    int fd_fifo = open(FIFO_RUTA, O_WRONLY);
    if (fd_fifo == -1) return;

    time_t tiempo = time(NULL);
    struct tm *tiempo_local = localtime(&tiempo);
    char mensaje[512];

    int longitud = snprintf( mensaje,sizeof(mensaje),
        "[%04d-%02d-%02d %02d:%02d:%02d] copiado %s\n",
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