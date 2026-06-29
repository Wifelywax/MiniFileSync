#ifndef UTILIDADES_IPC_H
#define UTILIDADES_IPC_H

#include <semaphore.h>
/*Estructura */
struct stats{
long archivos_copiados;
long bytes_copiados;
long errores;
};

/*Variables externas (estadisticas)*/
extern struct stats *estadisticas_globales;
extern sem_t *estadisticas_semaforos;

/*Memoria compartida,semaforos,limpieza*/
void inicializar_memoria_compartida(void);
void configurar_semaforos(void); 
void cerrar_limpiar_ipc(void);

#endif