#include "main.h"
#include "watek_komunikacyjny.h"
#include "watek_glowny.h"
#include "monitor.h"
/* wątki */
#include <pthread.h>

/* sem_init sem_destroy sem_post sem_wait */
//#include <semaphore.h>
/* flagi dla open */
//#include <fcntl.h>

state_t stan=InSetup;
volatile char end = FALSE;
int size,rank; /* nie trzeba zerować, bo zmienna globalna statyczna */
MPI_Datatype MPI_PAKIET_T;
pthread_t threadKom;

int lamport; // zegar Lamporta

int member_count[20] = {7, 25, 5, 13, 23, 11, 23, 13, 23, 24, 22, 5, 25, 16, 12, 17, 12, 25, 9, 7};
int team_size;
int combined_team_size;
int setup;

int total_desks = 30;
int total_rooms = 2;
int total_pads = 1;

int current_tables_taken = 0;
int current_rooms_taken = 0;
int current_pads_taken = 0;

int table[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int room[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int launchpad[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t teamMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lamportMut = PTHREAD_MUTEX_INITIALIZER;

void check_thread_support(int provided)
{
    debug("Started Checking Threads");
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    debug("Started Checking Threads");
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
	    MPI_Finalize();
	    exit(-1);
	    break;
        case MPI_THREAD_FUNNELED: 
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	    break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	    break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
	    break;
        default: printf("Nikt nic nie wie\n");
    }
}

/* srprawdza, czy są wątki, tworzy typ MPI_PAKIET_T
*/
void inicjuj(int *argc, char ***argv)
{
    int provided = 0;
    debug("Start of INIT");
    MPI_Init_thread(argc, argv,MPI_THREAD_MULTIPLE, &provided);
    debug("Po MPU_Init");
    check_thread_support(provided);
    debug("Threads Checked");


    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    const int nitems=4; /* bo packet_t ma trzy pola */
    int       blocklengths[4] = {1,1,1,1};
    MPI_Datatype typy[4] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[4]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);
    offsets[3] = offsetof(packet_t, tables);

    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);
	lamport = 0;
    team_size = member_count[rank];
    combined_team_size = member_count[rank];
    setup = 1;
    current_rooms_taken = size;
    current_pads_taken = size;


    pthread_create( &threadKom, NULL, startKomWatek , 0);
    //if (rank==0) {
	//pthread_create( &threadMon, NULL, startMonitor, 0);
    //}
    debug("jestem");
}

/* usunięcie zamkków, czeka, aż zakończy się drugi wątek, zwalnia przydzielony typ MPI_PAKIET_T
   wywoływane w funkcji main przed końcem
*/
void finalizuj()
{
    pthread_mutex_destroy( &stateMut);
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(threadKom,NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}


/* opis patrz main.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;
	pthread_mutex_lock(&lamportMut);
	lamport++;
	pkt->ts = lamport;
	pthread_mutex_unlock(&lamportMut);
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(pkt);
}

// void changeTallow( int newTallow )
// {
//     pthread_mutex_lock( &tallowMut );
//     if (stan==InFinish) { 
// 	pthread_mutex_unlock( &tallowMut );
//         return;
//     }
//     tallow += newTallow;
//     pthread_mutex_unlock( &tallowMut );
// }

void increaseTeamSize(int value){
    pthread_mutex_lock( &teamMut );
    combined_team_size = combined_team_size + value;
    setup += 1;
    current_tables_taken = combined_team_size;
    pthread_mutex_unlock( &teamMut );
    return;
}

void changeState( state_t newState )
{
    pthread_mutex_lock( &stateMut );
    if (stan==InFinish) { 
	pthread_mutex_unlock( &stateMut );
        return;
    }
    stan = newState;
    pthread_mutex_unlock( &stateMut );
}

int main(int argc, char **argv)
{
    /* Tworzenie wątków, inicjalizacja itp */
    
    inicjuj(&argc,&argv); // tworzy wątek komunikacyjny w "watek_komunikacyjny.c"
    
    setup = 1; // zebraliśmy dane od siebie
	
    mainLoop();          // w pliku "watek_glowny.c"

    finalizuj();
    return 0;
}

