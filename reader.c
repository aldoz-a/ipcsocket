// reader.c - main processo figlio
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "data.h"

// funzione main()
int main(int argc, char *argv[])
{
    // creo il socket in modo IPC e Datagram
    printf("processo %d partito\n", getpid());
    int sock;
    if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        // errore di creazione
        printf("%s: non posso creare il socket (%s)\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // prepara la struttura sockaddr_un per questo reader (è un server)
    struct sockaddr_un reader;
    memset(&reader, 0, sizeof(struct sockaddr_un));
    reader.sun_family = AF_UNIX;
    strcpy(reader.sun_path, IPCS_PATH);

    // associa l'indirizzo del reader al socket (questo crea il file IPCS_PATH)
    if (bind(sock, (struct sockaddr *)&reader, sizeof(reader)) == -1) {
        // errore di bind
        printf("%s: errore di bind (%s)", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // set clock e time per calcolare il tempo di CPU e il tempo di sistema
	clock_t t_start = clock();
    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);

    // loop di lettura messaggi dal writer
    Data my_data;
    while (recv(sock, &my_data, sizeof(my_data), 0) != -1) {
        // test index per forzare l'uscita
        if (my_data.index == N_MESSAGES) {
            // get clock e time per calcolare il tempo di CPU e il tempo di sistema
            clock_t t_end = clock();
            double t_passed = ((double)(t_end - t_start)) / CLOCKS_PER_SEC;
            struct timeval tv_end, tv_elapsed;
            gettimeofday(&tv_end, NULL);
            timersub(&tv_end, &tv_start, &tv_elapsed);

            // il processo chiude e cancella il socket ed esce per indice raggiunto
            printf("reader: ultimo messaggio ricevuto: %s\n", my_data.text);
            printf("processo %d terminato (index=%ld CPU time elapsed: %.3f - total time elapsed:%ld.%ld)\n",
                   getpid(), my_data.index, t_passed, tv_elapsed.tv_sec, tv_elapsed.tv_usec / 1000);
            close(sock);
            unlink(IPCS_PATH);
            exit(EXIT_SUCCESS);
        }
    }

    // il processo chiude e cancella il socket ed esce per altro motivo (errore)
    printf("processo %d terminato con errore (%s)\n", getpid(), strerror(errno));
    close(sock);
    unlink(IPCS_PATH);
    exit(EXIT_FAILURE);
}
