#ifndef GLOBALH
#define GLOBALH

#define _GNU_SOURCE
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
/* odkomentować, jeżeli się chce DEBUGI */
#define DEBUG 
/* boolean */
#define TRUE 1
#define FALSE 0

/* używane w wątku głównym, determinuje jak często i na jak długo zmieniają się stany */
#define STATE_CHANGE_PROB 50
#define SEC_IN_STATE 2

#define ROOT 0

/* stany procesu */
 // typedef enum {InRun, InMonitor, InSend, InFinish, InState} state_t;
typedef enum {InSetup, FinishedSetup,
            InWaitForTables, InTables, FinishedTables,
            InWaitForRoom, InRoom, FinishedRoom,
            InWaitForLaunchPad, InLaunchPad, FinishedLaunchPad,
            InWaitForTable, InTable, FinishedTable,
            InFinish} state_t;
extern state_t stan;
extern int rank;
extern int size;
extern int table[32];
extern int room[32];
extern int launchpad[32];
extern int member_count[20];


// /* Ile mamy łoju na składzie? */
// extern int tallow;

// /* stan globalny wykryty przez monitor */
// extern int globalState;
// /* ilu już odpowiedziało na GIVEMESTATE */
// extern int numberReceived;
// /* ilu się zatrzymało */
// extern int stopped;

extern int team_size;
extern int combined_team_size;
extern int setup;
extern int total_desks;
extern int total_rooms;
extern int total_pads;
extern int current_tables_taken;
extern int current_rooms_taken;
extern int current_pads_taken;
extern int my_request;
extern int reqTime;

/* zegar Lamporta*/
extern int lamport;

pthread_mutex_t lamportMut;
pthread_mutex_t stateMut;
pthread_mutex_t teamMut;

/* to może przeniesiemy do global... */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;      /* pole nie przesyłane, ale ustawiane w main_loop */
 
    int data;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
    int tables;
} packet_t;
extern MPI_Datatype MPI_PAKIET_T;

/* Typy wiadomości */
// #define FINISH 1
// #define TALLOWTRANSPORT 2
// #define INRUN 3
// #define INMONITOR 4
// #define GIVEMESTATE 5
// #define STATE 6
// #define STOP 7
// #define STOPPED 8

#define REQUEST 1
#define ACK 2


#define INIT 255


/* Strefy krytyczne */

#define DESK 1
#define ROOM 2
#define LAUNCHPAD 3


/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
					   "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape. 
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów
                                            
*/
#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d], %d: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, my_request, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#define P_WHITE printf("%c[%d;%dm",27,1,37);
#define P_BLACK printf("%c[%d;%dm",27,1,30);
#define P_RED printf("%c[%d;%dm",27,1,31);
#define P_GREEN printf("%c[%d;%dm",27,1,33);
#define P_BLUE printf("%c[%d;%dm",27,1,34);
#define P_MAGENTA printf("%c[%d;%dm",27,1,35);
#define P_CYAN printf("%c[%d;%d;%dm",27,1,36);
#define P_SET(X) printf("%c[%d;%dm",27,1,31+(6+X)%7);
#define P_CLR printf("%c[%d;%dm",27,0,37);

/* printf ale z kolorkami i automatycznym wyświetlaniem RANK. Patrz debug wyżej po szczegóły, jak działa ustawianie kolorków */
#define println(FORMAT, ...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);
void changeState( state_t );
//void changeTallow( int );
void increaseTeamSize( int );
void updateClock(int newClock);
#endif
