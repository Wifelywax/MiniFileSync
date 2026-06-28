#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include "../include/utilidades_ipc.h"

#define SHM_NAME "/shm_backup_stats"
#define SEM_NAME "/sem_backup_stats"

struct stats *estadisticas_globales = NULL;
sem_t *estadisticas_semaforos = NULL;

void inicializar_memoria_compartida(void){

    shm_unlink(SHM_NAME);

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1){
        perror("Error en shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, sizeof(struct stats)) == -1){
        perror("Error en ftruncate");
        exit(EXIT_FAILURE);
    }

    estadisticas_globales = mmap(NULL, sizeof(struct stats), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (estadisticas_globales == MAP_FAILED){
        perror("Error en mmap");
        exit(EXIT_FAILURE);
    }

    estadisticas_globales->archivos_copiados = 0;
    estadisticas_globales->bytes_copiados = 0;
    estadisticas_globales->errores = 0;
}

void configurar_semaforos(void) {
    sem_unlink(SEM_NAME);

    estadisticas_semaforos = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (estadisticas_semaforos == SEM_FAILED){
        perror("Error en sem_open");
        exit(EXIT_FAILURE);
    }
}

void cerrar_limpiar_ipc(void){
    munmap(estadisticas_globales, sizeof(struct stats));
    shm_unlink(SHM_NAME);
    sem_close(estadisticas_semaforos);
    sem_unlink(SEM_NAME);
}
 
