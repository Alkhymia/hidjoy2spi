#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include "SpiBus.h"
#include "spi.h"
#include "Xpad.h"

int fd = -1;
int running = 1;

typedef struct {
    XpadReport_Data_t (*supplier)(void);

    void (*callback)(XpadRumble_t *);
} t_supplier_and_callback;

void *readWriteSpi(void *args) {
    t_supplier_and_callback *holder = args;

    spi_init(fd);
    while (running) {
        XpadReport_Data_t reportData = (holder->supplier)();
        XpadRumble_t rumble;

        transfer(fd, (uint8_t *) &reportData, (uint8_t *) &rumble);

        (holder->callback)(&rumble);

        usleep(3000);
    }
}

void startSpiThread(char *file, XpadReport_Data_t (*supplier)(void), void (*callback)(XpadRumble_t *)) {
    pthread_t threadId;
    fd = open(file, O_RDONLY);

    printf("\nOpening SPI: %s\n", file);
    if (fd == -1) {
        printf("SPI file not found or permission denied\n");
        return;
    }

    t_supplier_and_callback arg;
    arg.callback = callback;
    arg.supplier = supplier;

    pthread_create(&threadId, NULL, readWriteSpi, &arg);
    pthread_join(threadId, NULL);
}