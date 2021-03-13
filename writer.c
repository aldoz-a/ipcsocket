// writer.c - main processo figlio
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
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

    // prepara la struttura sockaddr_un per il reader (è un server) remoto
    struct sockaddr_un reader;
    memset(&reader, 0, sizeof(struct sockaddr_un));
    reader.sun_family = AF_UNIX;
    strcpy(reader.sun_path, IPCS_PATH);

    // loop di scrittura messaggi per il reader
    Data my_data;
    my_data.index = 0;
    do {
        // test index per forzare l'uscita
        if (my_data.index == N_MESSAGES) {
            // il processo chiude il socket ed esce per indice raggiunto
            printf("processo %d terminato (text=%s index=%ld)\n", getpid(), my_data.text, my_data.index);
            close(sock);
            exit(EXIT_SUCCESS);
        }

        // compongo il messaggio e lo invio
        my_data.index++;
        snprintf(my_data.text, sizeof(my_data.text), "un-messaggio-di-test:%ld", my_data.index);
    } while (sendto(sock, &my_data, sizeof(my_data), 0, (struct sockaddr *)&reader, sizeof(reader)) != -1);

    // il processo chiude il socket ed esce per altro motivo (errore)
    printf("processo %d terminato con errore (%s)\n", getpid(), strerror(errno));
    close(sock);
    exit(EXIT_FAILURE);
}
