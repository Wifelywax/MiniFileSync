# MiniFileSync

El proyecto nos permite simular lo que es un sistema de sincronizacion de archivos, en donde se aplico una transformacion a demonio que se ejecuta en segundo plano y que a la par detecta si hubo alguna modificacion para crear una copia en el backup que se tiene.

## Cararteristicas

* Multiprocesos: Hay el monitor que es el que maneja todo y los workers que se crearon con fork(). 
* Demonio: Se pone en segundo plano usando el setsid y el chdir
* Sincronizacion: Solo copia de nuevo a los archivos que presentan un cambio, que se ve cuando comparan los metadatos
* IPC: Se da tareas mediante las tuberias.
* Concurrencia: Evita la condicion de carrera gracias al uso de semaforos. 
* Tolerancia a WSL: Solventar el drift time con el uso de utime porque los archivos se copiaban mas de 3 veces.

## Requisitos 

* Compilador
* Uso de make
* Librerias estandar de C 

## Compilacion

En la raiz del proyecto se debe ejecutar el "make" para compilar. Las veces que se probo se lo hizo de la siguiente manera: 
* make
* ./scan ./src
para saber si realmente copiaba y notaba los cambios de los archivos dentro del src.
Y para terminarlo definitivamente se usa:
* pkill -f scan