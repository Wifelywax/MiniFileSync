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
    exit(EXIT_FAILURE);
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

int main(int argc, char *argv[]){
    //Verificar si se ejecuta de forma correcta(./scan <directorio>)
    if(argc != 2){
        fprintf(stderr, "Debes usarlo de la siguiente forma: %s<directorio>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *directorio_origen= argv[1];
    
    inicializar_memoria_compartida();
    configurar_semaforos();
    crear_logger();

    printf("Iniciando el monitor de directorios:\n");
    printf("Escaneando el directorio: %s\n", directorio_origen);

    //Ejecucion del escaner
    escanear_directorio(directorio_origen);

    //Mostrar los resultados con sus parametros
    printf("Total de archivos:%d\n", total_archivos);
    
    int tuberia[NUM_WORKERS][2];
    pid_t pids[NUM_WORKERS];
    printf("Creando %d worker\n", NUM_WORKERS);


    for(int i=0; i<NUM_WORKERS;i++){
        if(pipe(tuberia[i])==-1){
            perror("Error al crear la tubería");
            exit(EXIT_FAILURE);
        }
    

    pids[i]=fork();
    

    if(pids[i]==-1){
        perror("Error al hacer fork");
        exit(EXIT_FAILURE);
    }
    
    if(pids[i]==0){
        close(tuberia[i][1]);
        char ruta_recibida[PATH_MAX];

        while(read(tuberia[i][0], ruta_recibida, PATH_MAX) > 0){
            procesar_archivo(ruta_recibida,i+1);
        }

        close(tuberia[i][0]);
        exit(EXIT_SUCCESS);
    } else {
        close(tuberia[i][0]);
    }

}

    for(int i=0;i<total_archivos;i++){
        
        int id_worker= i %NUM_WORKERS;
        write(tuberia[id_worker][1], memoria_archivos[i].ruta, PATH_MAX);
    }

    for(int i = 0; i < NUM_WORKERS; i++){
        close(tuberia[i][1]);
    }

    for(int i=0; i < NUM_WORKERS;i++){
        waitpid(pids[i],NULL,0);
    }

    printf("\n Proceso de los workers terminado\n");

    printf("\n=== ESTADÍSTICAS FINALES DEL BACKUP ===\n");
    printf("Archivos copiados/actualizados: %ld\n", estadisticas_globales->archivos_copiados);
    printf("Total de bytes transferidos: %ld bytes\n", estadisticas_globales->bytes_copiados);
    printf("=======================================\n");

    cerrar_limpiar_ipc();

    return EXIT_SUCCESS;
}