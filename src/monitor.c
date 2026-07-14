#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <linux/limits.h>
#include "../include/escaner.h"
#include "../include/workers.h"
#include "../include/utilidades_ipc.h"
#include "../include/registro.h"

#define  NUM_WORKERS 3

char ruta_base_absoluta[PATH_MAX];

extern Metadatos memoria_archivos[];
extern int total_archivos;

void convertir_demonio(){

pid_t pid=fork();

if (pid<0){
    perror("Error al hacer fork");
    exit(EXIT_FAILURE);
}

if(pid>0){
    printf("[Monitor] en segundo plano");
}
 
if(setsid() < 0){
    //perror("Error al ejecutar setsid");
}

if(chdir("/")<0){
    perror("Error al cambiar directorio raiz");
    exit(EXIT_FAILURE);
}

int fd_nulo = open("/dev/null",O_RDWR);
if(fd_nulo !=1){
    dup2(fd_nulo, STDIN_FILENO);
    dup2(fd_nulo, STDOUT_FILENO);
    dup2(fd_nulo, STDERR_FILENO);

    close(fd_nulo);
 }

}

int main(int argc, char *argv[]) {
    // 1. Verificar argumentos antes de hacer cualquier cambio
    if (argc != 2) {
        fprintf(stderr, "Uso correcto: %s <directorio>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    if (getcwd(ruta_base_absoluta, sizeof(ruta_base_absoluta)) == NULL) {
        perror("Error al obtener el directorio actual");
        exit(EXIT_FAILURE);
    }

    char directorio_objetivo[PATH_MAX];
    if (realpath(argv[1], directorio_objetivo) == NULL) {
        perror("Error al resolver la ruta absoluta del directorio");
        exit(EXIT_FAILURE);
    }
    // Inicializar Memoria compartida, Semáforos y Logger
    inicializar_memoria_compartida();
    configurar_semaforos();
    crear_logger();
    //Imprimir datos
    total_archivos = 0;
    escanear_directorio(directorio_objetivo); // Escanear una vez
   
    printf("\n=== METADATOS RECOPILADOS EN MEMORIA ===\n");
    for (int i = 0; i < total_archivos; i++) {
        // Extraer nombre del archivo de la ruta 
        char *nombre = strrchr(memoria_archivos[i].ruta, '/');
        nombre = (nombre) ? nombre + 1 : memoria_archivos[i].ruta;
        // Imprimir Inodo, Tamaño y los Permisos
        printf("[Worker %d]| - %s | Inodo: %lu | Tamaño: %ld bytes | Permisos: %o\n",
               (i % NUM_WORKERS) + 1,
               nombre,
               (unsigned long)memoria_archivos[i].num_inodo,
               (long)memoria_archivos[i].tamanio,
               memoria_archivos[i].permisos & 0777); 
    }
    printf("========================================\n\n");
    //Transformar en Demonio
    convertir_demonio();
    //Bucle Infinito 
    while (1) {
       
        total_archivos = 0; 
       
        escanear_directorio(directorio_objetivo);
        // Si hay archivos, se asigna un worker
        if (total_archivos > 0) {
            int tuberia[NUM_WORKERS][2];
            pid_t pids[NUM_WORKERS];
            // Crear las tuberías y los procesos trabajadores
            for (int i = 0; i < NUM_WORKERS; i++) {
                if (pipe(tuberia[i]) == -1) {
                    exit(EXIT_FAILURE);
                }

                pids[i] = fork();
                if (pids[i] == 0) {
                    //Worker 
                    close(tuberia[i][1]); // Cierra escritura
                    char ruta_recibida[PATH_MAX];

                    while (read(tuberia[i][0], ruta_recibida, PATH_MAX) > 0) {
                        procesar_archivo(ruta_recibida, i + 1);
                    }
                    close(tuberia[i][0]);
                    exit(EXIT_SUCCESS);
                } else {
                    //Monitor 
                    close(tuberia[i][0]); // Cierra lectura
                }
            }
            // Distribuir de forma equitativa
            for (int i = 0; i < total_archivos; i++) {
                int id_worker_asignado = i % NUM_WORKERS;
                write(tuberia[id_worker_asignado][1], memoria_archivos[i].ruta, PATH_MAX);
            }
            // Cerrar extremos de escritura
            for (int i = 0; i < NUM_WORKERS; i++) {
                close(tuberia[i][1]);
            }
            // Esperar el fin del trabajo
            for (int i = 0; i < NUM_WORKERS; i++) {
                waitpid(pids[i], NULL, 0);
            }
        }
        sleep(5);
    }
    cerrar_limpiar_ipc();
    return EXIT_SUCCESS;
} //hola