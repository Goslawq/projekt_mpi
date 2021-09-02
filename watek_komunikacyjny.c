#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	//debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		pthread_mutex_lock(&lamportMut);
		if (lamport > pakiet.ts){
			lamport++;
		}else{
			lamport = pakiet.ts;
			lamport++;
		}
		pthread_mutex_unlock(&lamportMut);


		switch ( status.MPI_TAG){
			case REQUEST:
				//debug("Got REQUEST");
				switch (pakiet.data){
					case DESK:
						if (stan == InWaitForTables ||  stan == InWaitForTable ){
							if (pakiet.ts < my_request) {
								pakiet.tables = team_size;
								sendPacket(&pakiet, pakiet.src, ACK);
							}else{
								table[pakiet.src] = 1;
							} 
						}else if(stan == InTables || stan == InTable){
							table[pakiet.src] = 1;
						}else {
							pakiet.tables = team_size;
							sendPacket(&pakiet, pakiet.src, ACK);
						}
					break;
					case ROOM:
						if (stan == InWaitForRoom ){
							if (pakiet.ts < my_request) {
								sendPacket(&pakiet, pakiet.src, ACK);
							}else{
								room[pakiet.src] = 1;
							} 
						}else if(stan == InRoom){
							room[pakiet.src] = 1;
						}else {
							sendPacket(&pakiet, pakiet.src, ACK);
						}
					break;
					case LAUNCHPAD:
						if (stan == InWaitForLaunchPad){
							if (pakiet.ts < my_request) {
								sendPacket(&pakiet, pakiet.src, ACK);
							}else{
								launchpad[pakiet.src] = 1;
							} 
						}else if(stan == InLaunchPad){
							launchpad[pakiet.src] = 1;
						}else {
							sendPacket(&pakiet, pakiet.src, ACK);
						}
					break;
				}
			break;
			case ACK:
				//debug("Got ACK");
				switch (pakiet.data){
					case DESK:
						if (stan == InWaitForTables || stan == InWaitForTable){
							pthread_mutex_lock( &teamMut );
							current_tables_taken = current_tables_taken - pakiet.tables;
							pthread_mutex_unlock( &teamMut );
							table[pakiet.src] = 0;
							debug("Czekam aż będzie dostępne więcej niż %i a jest %i", total_desks, current_tables_taken);
							if (current_tables_taken <= total_desks){
								if (stan == InWaitForTables){
									changeState(InTables);
								}else{
									changeState(InTable);
								}
							}
						}
					break;
					case ROOM:
						if (stan == InWaitForRoom){
							pthread_mutex_lock( &teamMut );
							current_rooms_taken--;
							pthread_mutex_unlock( &teamMut );
							room[pakiet.src] = 0;
							if (current_rooms_taken <= total_rooms){
								changeState(InRoom);
							}
						}
					break;
					case LAUNCHPAD:
						if (stan == InWaitForLaunchPad){
							debug("Zajętych jest %i a może być %i", current_pads_taken, total_pads);
							pthread_mutex_lock( &teamMut );
							current_pads_taken--;
							pthread_mutex_unlock( &teamMut );
							launchpad[pakiet.src] = 0;
							if (current_pads_taken <= total_pads){
								changeState(InLaunchPad);
							}
						}
					break;
				}
			break;
			case INIT:
				//debug("Got INIT");
				increaseTeamSize(pakiet.data);
			break;
		}

        /*switch ( status.MPI_TAG ) {
	    case FINISH: 
                changeState(InFinish);
	    break;
	    case TALLOWTRANSPORT: 
                changeTallow( pakiet.data);
                debug("Dostałem wiadomość od %d z danymi %d",pakiet.src, pakiet.data);
	    break;
	    case GIVEMESTATE: 
                pakiet.data = tallow;
                sendPacket(&pakiet, ROOT, STATE);
                debug("Wysyłam mój stan do monitora: %d funtów łoju na składzie!", tallow);
				changeState( InRun );
	    break;
            case STATE:
                numberReceived++;
                globalState += pakiet.data;
                if (numberReceived > size-1) {
                    debug("W magazynach mamy %d funtów łoju.", globalState);
                } 
            break;
	    case INMONITOR: 
                changeState( InMonitor );
                debug("Od tej chwili czekam na polecenia od monitora");
	    break;
	    case INRUN: 
                changeState( InRun );
                debug("Od tej chwili decyzję podejmuję autonomicznie i losowo");
	    break;
		case STOP:
				debug("Dostałem polecenie stopu");
				while (stan == InSend){
					;
				}
				changeState ( InState );
				sendPacket(&pakiet, ROOT, STOPPED);
				debug("Spełniłem polecenie stopu");
				
		break;
		case STOPPED:
				stopped++;
		break;
	    default:
	    break;
        }*/
    }
}
