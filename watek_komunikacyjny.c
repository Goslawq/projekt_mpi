#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
//    int new_lamport;
    int is_message = FALSE;
    int lowerPriority = TRUE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	//debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

//		pthread_mutex_lock(&lamportMut);
//		if (my_request > pakiet.ts){
//			new_lamport++;
//		}else{
//			new_lamport = pakiet.ts;
//			new_lamport++;
//		}
//		lamport = new_lamport;
//		pthread_mutex_unlock(&lamportMut);

	updateClock(pakiet.ts);
		switch ( status.MPI_TAG){
			case REQUEST:
				//debug("Got REQUEST");
				if(reqTime == pakiet.ts){
					if(pakiet.src < rank){
						lowerPriority = FALSE;
					}else{
						lowerPriority = TRUE;
					}
				}else{
					if(reqTime < pakiet.ts){
						lowerPriority = FALSE;
					}else{
						lowerPriority = TRUE;
					}
				}
				switch (pakiet.data){
					case DESK:
						if (stan == InWaitForTables || stan == InWaitForTable ){
					//		debug("BIURKA: moj zegar %i, zegar przych %i", pakiet.ts, reqTime);
							if (lowerPriority) {
								pakiet.tables = team_size;
								sendPacket(&pakiet, pakiet.src, ACK);
								updateClock(my_request);
								//my_request = new_lamport;
							}else{
			//					debug("Bede czekal na odp desk");
								table[pakiet.src] = 1;
							} 
						}else if(stan == InTables || stan == InTable){
			//				debug("Bede czekal na odp DESK od %i", pakiet.src);
							table[pakiet.src] = 1;
						}else {
							pakiet.tables = team_size;
							sendPacket(&pakiet, pakiet.src, ACK);
							updateClock(my_request);
							//my_request = new_lamport;
						}
					break;
					case ROOM:
						if (stan == InWaitForRoom ){
                                          //              debug("SALKI: moj zegar %i, zegar przych %i", pakiet.ts, reqTime);
							if (lowerPriority){
								updateClock(my_request);
								//my_request = new_lamport;
								sendPacket(&pakiet, pakiet.src, ACK);
							}else{
							//	debug("Bede czekal na odp ROOM od %i", pakiet.src);
								room[pakiet.src] = 1;
							} 
						}else if(stan == InRoom){
							room[pakiet.src] = 1;
							//debug("Bede czekal na odp ROOM od %i", pakiet.src);
						}else {                      
		      					updateClock(my_request);
                                                        //my_request = lamport;
							sendPacket(&pakiet, pakiet.src, ACK);
						}
					break;
					case LAUNCHPAD:
						if (stan == InWaitForLaunchPad){
                                            //            debug("PAS: moj zegar %i, zegar przych %i", pakiet.ts, reqTime);
							if (lowerPriority) {
								updateClock(my_request);
								//my_request = new_lamport;
								sendPacket(&pakiet, pakiet.src, ACK);
							}else{
							//	debug("Bede czekal na odp LAUNCH od %i", pakiet.src);
								launchpad[pakiet.src] = 1;
							} 
						}else if(stan == InLaunchPad){
							//debug("Bede czekal na odp LAUNCH od %i", pakiet.src);
							launchpad[pakiet.src] = 1;
						}else {
		                                        updateClock(my_request);
                                                        //my_request = lamport;
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
						//	table[pakiet.src] = 0;
							debug("Czekam aż będzie dostępne więcej niż %i a jest %i", total_desks, current_tables_taken);
							if (current_tables_taken <= total_desks){
								if (stan == InWaitForTables){
									debug("Zajmuję biurka");
									changeState(InTables);
								}else{
									debug("Zajmuję biurko");
									changeState(InTable);
								}
							}
                                                        pthread_mutex_unlock( &teamMut );
						}
						
					break;
					case ROOM:
						if (stan == InWaitForRoom){
							pthread_mutex_lock( &teamMut );
							current_rooms_taken--;
						//	room[pakiet.src] = 0;
							debug("Zajetych pokojow %i, a moze byc %i",current_rooms_taken, total_rooms);
							if (current_rooms_taken <= total_rooms){
								//room[pakiet.src] = 0;
								debug("Zajmuję pokój");
								changeState(InRoom);
							}
							pthread_mutex_unlock( &teamMut );
						}
					break;
					case LAUNCHPAD:
						if (stan == InWaitForLaunchPad){
							debug("Zajętych jest %i a może być %i", current_pads_taken-1, total_pads);
							pthread_mutex_lock( &teamMut );
							current_pads_taken--;
						//	launchpad[pakiet.src] = 0;
							if (current_pads_taken <= total_pads){
								debug("Zajmuję pole startowe");
								changeState(InLaunchPad);
							}
                                                        pthread_mutex_unlock( &teamMut );
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
