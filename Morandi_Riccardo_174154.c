/*

Morandi Riccardo
n° matr: 174154

Le modifiche che sono state apportate alla versione cartacea sono le seguenti:
-   Aggiunta dei commenti del programma che non avevo riportato nella versione cartacea;
-   Aggiunta degli #include delle librerie necessarie per l'esecuzione del programma;
-   Aggiunta di "\n" al termine di ogni printf per permettere una buona lettura dell'esecuzione del codice;
-   Aggiunta una funzione pausa per poter creare delle attese random e poter simulare al meglio lo sviluppo della partita;
-   Aggiunta di sleep sempre per ottenere una maggior chiarezza sull'output nel terminale;
-   Aggiunta la funzione main per la creazione dei thread e per eseguire il codice.

*/

#include <stdio.h>                      // Includo le librerie necessarie per utilizzare pthread, semafori, sleep e rand
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define NUMERO_GIOCATORI 2              // Definisco il numero dei giocatori       

struct bandierine_t {                   // Definisco la struttura del gestore delle risorse che riguardano il gioco

    sem_t mutex;                        // Semaforo di mutua esclusione per proteggere l'accesso alle risorse condivise

    sem_t s_attendi_via;                // Semaforo di attesa per i giocatori in partenza
    sem_t s_attendi_giocatori;          // Semaforo di attesa del giudice nel caso i giocatori non siano pronti
    sem_t s_attendi_fine;               // Semaforo di attesa del giudice della fine della partita

    int giocatori_pronti;               // Giocatori in coda al semaforo s_attendi_via
    int bandierina;                     // Valore intero che rappresenta la bandierina in mano al giudice (1 se non è stata presa, 0 se è stata presa)
    int gioco_finito;                   // Valore intero che definisce se la partita si è conclusa (1 se si è conclusa, 0 se non si è ancora conclusa)
    int vincitore;                      // Valore intero che rappresenta il numero del giocatore vincente
    int giudice_pronto;                 // Valore intero che indica se il giudice è bloccato sul semaforo s_attendi_giocatori

} bandierine;

void init_bandierine(struct bandierine_t* b) {       // Inizializzo la struttura del gestore

    sem_init(&b->mutex, 0, 1);                       // inizializzo il mutex verde

    sem_init(&b->s_attendi_via, 0, 0);               // Inizializzo tutti i semafori rimanenti rossi
    sem_init(&b->s_attendi_giocatori, 0, 0);
    sem_init(&b->s_attendi_fine, 0, 0);

    b->giocatori_pronti = 0;                         // Inizializzo i contatori a 0 tranne per la bandierina che all'inzio è uguale a 1 (non è ancora stata presa)
    b->bandierina = 1;
    b->giudice_pronto = 0;
    b->vincitore = 0;
    b->gioco_finito = 0;

}

void attendi_giocatori(struct bandierine_t* b) {    // Funzione di attesa del giudice che aspetta che i giocatori siano pronti

    sem_wait(&b->mutex);                            // Richiesta del semaforo di mutua esclusione per l'accesso alle risorse condivise

    if (b->giocatori_pronti == NUMERO_GIOCATORI) {  // Se i giocatori sono pronti eseguo una post sul mio semaforo di attesa per poter proseguire in seguito
        sem_post(&b->s_attendi_giocatori);
    }
    else {                                          // Se i giocatori non sono pronti imposto il contatore giudice_pronto = 1 per segnalare che sono in attesa e poi mi blocco
        b->giudice_pronto = 1;
    }
    printf("Il giudice è pronto!\n");
    sem_post(&b->mutex);                            // Post sul semaforo di mutua esclusione
    sem_wait(&b->s_attendi_giocatori);              // Se i giocatori sono pronti il giudice potrà proseguire, altrimenti il giudice attenderà qui

}

void attendi_il_via(struct bandierine_t* b, int n) {                    // Funzione di attesa dei giocatori prima del via

    sem_wait(&b->mutex);
    printf("Il giocatore %d è pronto!\n", n);                           // Segnalo che il giocatore n è pronto a partire
    b->giocatori_pronti++;                                              // Aumento il numero di giocatori pronti

    if (b->giocatori_pronti == NUMERO_GIOCATORI && b->giudice_pronto) { // Se il numero di giocatori pronti è uguale al numero dei giocatori e il giudice è in attesa allora 
                                                                        // permetto al giudice di proseguire
        sem_post(&b->s_attendi_giocatori);
    }
    sem_post(&b->mutex);
    sem_wait(&b->s_attendi_via);                                        // In ogni caso i giocatori attenderanno su questo semaforo il via della partita

}

void via(struct bandierine_t* b) {              // Funzione di inzio partita utilizzata dal giudice

    sem_wait(&b->mutex);
    printf("Via!\n");                           // Segnalo l'inizio della partita

    while (b->giocatori_pronti) {               // Finche ci sono giocatori bloccati in coda sul semaforo s_attendi_via li libero per permettere l'inizio della partita
        sem_post(&b->s_attendi_via);
        b->giocatori_pronti--;
    }
    sem_post(&b->mutex);
    sem_wait(&b->s_attendi_fine);               // Il giudice si ferma su questo semaforo ad attendere che la partita si sia conclusa


}

int bandierina_presa(struct bandierine_t* b, int n) {       //Funzione che rappresenta la presa della bandierina da parte di uno dei giocatori

    sem_wait(&b->mutex);
    int ba = 0;                                             // Inizializzo una variabile intera locale che indica il risultato della funzione

    if (b->bandierina) {                                    // Se la bandierina non è ancora stata presa segnalo che il giocatore n ha ottenuto la bandierina, cambio il risultato 
                                                            // nella variabile locale e nel gestore delle bandierine scrivo che la bandierina non è più in mano al giudice
        printf("Bandierina presa dal giocatore %d\n", n);
        b->bandierina = 0;
        ba++;
    }
    sem_post(&b->mutex);
    return ba;                                              // Restituisco il risultato della funzione (1 se ho ottenuto la bandierina, 0 se era già stata presa)

}

int sono_salvo(struct bandierine_t* b, int n) {          // Funzione che rappresenta l'arrivo in base da parte di un giocatore n

    sem_wait(&b->mutex);
    int result = 0;                                     // Inizializzo una variabile intera locale che indica il risultato della funzione

    if (b->gioco_finito == 0) {                         // Se la variabile gioco finito è uguale a 0 (partita ancora in corso) allora il giocatore n è salvo e vince! 
        b->gioco_finito = 1;                            // Imposto la variabile gioco finito uguale a 1 (partita finita)
        b->vincitore = n;                               // Imposto la variabile vincitore con il mio numero
        result = 1;                                     // Modifico il risultato della funzione
        sem_post(&b->s_attendi_fine);                   // Libero il giudice in attesa sul semaforo s_attendi_fine
    }
    sem_post(&b->mutex);
    return result;                                      // Restituisco il risultato della funzione (1 se sono salvo, 0 se la partita era finita)

}

int ti_ho_preso(struct bandierine_t* b, int n) {        // Funzione che rappresenta la presa dell'altro giocatore da parte del giocatore n

    sem_wait(&b->mutex);
    int result = 0;                                     // Inizializzo una variabile intera locale che indica il risultato della funzione

    if (b->gioco_finito == 0) {                         // Se la variabile gioco finito è uguale a 0 allora il giocatore n ha preso l'altro giocatore e vince!
        b->gioco_finito = 1;                            // Imposto la variabile gioco finito uguale a 1 (partita finita)
        b->vincitore = n;                               // Imposto la variabile vincitore con il mio numero
        result = 1;                                     // Modifico il risultato della funzione
        sem_post(&b->s_attendi_fine);                   // Libero il giudice in attesa sul semaforo s_attendi_fine
    }
    sem_post(&b->mutex);
    return result;                                      // Restituisco il risultato della funzione (1 se ho preso l'altro giocatore, 0 se la partita era finita)

}

int risultato_gioco(struct bandierine_t* b) {           // Funzione che ritorna il risultato della partita

    sem_wait(&b->mutex);
    int v = b->vincitore;                               // Inizializzo una variabile intera con il numero del giocatore vincente
    sem_post(&b->mutex);
    return v;                                           // Ritorno il numero del vincitore

}

void pausa(int n) {           // Funzione che permette di avere una pausa random in modo da poter simulare i tempi tra una azione di gioco e l'altra

    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % n + 1) * 1000000;
    nanosleep(&t, NULL);

}

void* giocatore(void* arg) {

    int numerogiocatore = (long)arg;
    attendi_il_via(&bandierine, numerogiocatore);
    pausa(10);                                                                        // Inserimento di una pausa
    if (bandierina_presa(&bandierine, numerogiocatore)) {
        pausa(10);                                                                    // Inserimento di una pausa
        if (sono_salvo(&bandierine, numerogiocatore)) printf("Salvo!\n");
    }
    else {
        pausa(10);                                                                    // Inserimento di una pausa
        if (ti_ho_preso(&bandierine, numerogiocatore)) printf("Ti ho preso!\n");
    }
    return 0;

}

void* giudice(void* arg) {

    attendi_giocatori(&bandierine);
    printf("Pronti... Attenti...\n");                               // Aggiunta di un printf per rappresentare al meglio la situazione
    sleep(1);                                                       // Attesa per simulare la partenza
    via(&bandierine);
    sleep(1);                                                       // Attesa per simulare la fine
    printf("Il vincitore è: %d\n", risultato_gioco(&bandierine));
    return 0;

}

int main(int argc, char const* argv[]) {                           // Funzione principale per la creazione dei thread

    pthread_attr_t a;
    pthread_t p;

    init_bandierine(&bandierine);                                  // Chiamata della inzializzazione del gestore bandierine
    srand(time(NULL));
    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, giocatore, (void*)1);                   // Creazione del thread giocatore 1
    pthread_create(&p, &a, giocatore, (void*)2);                   // Creazione del thread giocatore 2
    pthread_create(&p, &a, giudice, NULL);                         // Creazione del thread giudice


    pthread_attr_destroy(&a);
    sleep(3);

    return 0;
}
