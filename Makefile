CC = gcc
CFLAGS = -Wall -Wextra -g -I./include

LDFLAGS = -lrt -pthread

SRCS = src/monitor.c src/escaner.c src/workers.c src/registro.c src/utilidades_ipc.c

OBJS = $(SRCS:.c=.o)

TARGET = scan

all: $(TARGET) carpetas

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

carpetas:
	mkdir -p backup

clean:
	rm -f src/*.o $(TARGET)
	@echo "Limpieza completa"

clean_backup:
	rm -rf backup/*
	@echo "Carpeta de backup limpia"