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
    perror("Error al ejecutar setsid");
    //exit(EXIT_FAILURE);
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
    // 2. Resolver la ruta absoluta del directorio objetivo antes del chdir("/")
    char directorio_objetivo[PATH_MAX];
    if (realpath(argv[1], directorio_objetivo) == NULL) {
        perror("Error al resolver la ruta absoluta del directorio");
        exit(EXIT_FAILURE);
    }

    // 3. Inicializar las herramientas globales (Memoria compartida, Semáforos y Logger)
    inicializar_memoria_compartida();
    configurar_semaforos();
    crear_logger();

    //4. Imprimir datos
    total_archivos = 0;
    escanear_directorio(directorio_objetivo); // Escanear una vez
    
    printf("\n=== METADATOS RECOPILADOS EN MEMORIA ===\n");
    for (int i = 0; i < total_archivos; i++) {
        // Extraemos solo el nombre del archivo de la ruta larga para que se vea bonito
        char *nombre = strrchr(memoria_archivos[i].ruta, '/');
        nombre = (nombre) ? nombre + 1 : memoria_archivos[i].ruta;

        // Imprimimos el Inodo, Tamaño y los Permisos en formato octal (ej. 644)
        printf("- %s | Inodo: %lu | Tamaño: %ld bytes | Permisos: %o\n",
               nombre,
               (unsigned long)memoria_archivos[i].num_inodo,
               (long)memoria_archivos[i].tamanio,
               memoria_archivos[i].permisos & 0777); 
    }
    printf("========================================\n\n");

    // 5. Transformar el proceso principal en Demonio
    convertir_demonio();

    // 6. Bucle Infinito de Monitoreo Continuo
    while (1) {
        // RESETEAR CONTADOR: Vital para no arrastrar datos de la iteración de hace 5 segundos
        total_archivos = 0; 

        // Escanear el directorio objetivo de forma recursiva
        escanear_directorio(directorio_objetivo);

        // Si el escáner detectó archivos, procedemos con la asignación a los workers
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
                    // Código del Worker (Hijo)
                    close(tuberia[i][1]); // Cierra escritura
                    char ruta_recibida[PATH_MAX];

                    while (read(tuberia[i][0], ruta_recibida, PATH_MAX) > 0) {
                        procesar_archivo(ruta_recibida, i + 1);
                    }
                    close(tuberia[i][0]);
                    exit(EXIT_SUCCESS);
                } else {
                    // Código del Monitor (Padre)
                    close(tuberia[i][0]); // Cierra lectura
                }
            }

            // Distribuir de forma equitativa las rutas encontradas
            for (int i = 0; i < total_archivos; i++) {
                int id_worker_asignado = i % NUM_WORKERS;
                write(tuberia[id_worker_asignado][1], memoria_archivos[i].ruta, PATH_MAX);
            }

            // Indicar fin de transmisión cerrando los extremos de escritura
            for (int i = 0; i < NUM_WORKERS; i++) {
                close(tuberia[i][1]);
            }

            // Esperar que la ronda actual de workers termine por completo
            for (int i = 0; i < NUM_WORKERS; i++) {
                waitpid(pids[i], NULL, 0);
            }
        }

        // Esperar 5 segundos exactos antes de la siguiente revisión completa
        sleep(5);
    }

    // Liberacion formal de recursos (Teórica, el demonio corre hasta ser matado)
    cerrar_limpiar_ipc();
    return EXIT_SUCCESS;
}