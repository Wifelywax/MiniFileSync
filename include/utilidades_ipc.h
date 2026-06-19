#ifndef UTILIDADES_IPC_H
#define UTILIDADES_IPC_H

struct stats{
long archivos_copiados;
long bytes_copiados;
long errores;
};

void inicializar_memoria_compartida(void);
void configurar_semaforos(void);

#endif