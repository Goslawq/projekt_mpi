#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    packet_t *pkt = malloc(sizeof(packet_t));
    while (stan != InFinish){
        if (stan == InSetup){
            if (setup < size){
                continue;
            } else if (setup == size){
				
                changeState( FinishedSetup );
            }
        } else if (stan == FinishedSetup){
            pkt->data = DESK;
            for (int i=0; i<size;i++){
                sendPacket(pkt, i, REQUEST);
            }
				pthread_mutex_lock( &teamMut );
				int my_request = lamport;
				pthread_mutex_unlock( &teamMut );

            changeState(InWaitForTables);
        //} else if (stan == InWaitForTables){
        //    sleep(SEC_IN_STATE);
        } else if (stan == InTables){
            sleep(SEC_IN_STATE);
            debug("Już wiemy co dzisiaj wysadzimy");
            sleep(SEC_IN_STATE);
            changeState(FinishedTables);
        } else if (stan == FinishedTables){
            pkt->data = DESK;
            pkt->tables = team_size;
            for (int i=0; i<size;i++){
                if (table[i] == 1){
                    sendPacket(pkt, i, ACK);
                }
            }
            
            pthread_mutex_lock( &teamMut );
            current_tables_taken = combined_team_size;
            current_rooms_taken = size;
            pthread_mutex_unlock( &teamMut );
            pkt->data = ROOM;
            for (int i=0; i<size;i++){
                sendPacket(pkt, i, REQUEST);
            }
				pthread_mutex_lock( &teamMut );
				int my_request = lamport;
				pthread_mutex_unlock( &teamMut );
            changeState(InWaitForRoom);
        } else if (stan == InRoom){
            sleep(SEC_IN_STATE);
            debug("Z przyjemnością ogłaszamy że tym razem start się uda");
            sleep(SEC_IN_STATE);
            changeState(FinishedRoom);
        }else if (stan == FinishedRoom){
			//debug("Started FinishedRoom")
            pkt->data = ROOM;
            for (int i=0; i<size;i++){
				//debug("FinishedRoom loop %i", i);
                if (room[i] == 1){
					debug("FinishedRoom loop %i, sending packet", i);
                    sendPacket(pkt, i, ACK);
                }
            }
			//debug("Przed free");
            
			//debug("Sent FinishedRoom Requests")
            pthread_mutex_lock( &teamMut );
            current_rooms_taken = size;
            current_pads_taken = size;
			debug("mainthread c_p_t = %i", current_pads_taken);
            pthread_mutex_unlock( &teamMut );
            pkt->data = LAUNCHPAD;
            for (int i=0; i<size;i++){
                sendPacket(pkt, i, REQUEST);
            }
				pthread_mutex_lock( &teamMut );
				int my_request = lamport;
				pthread_mutex_unlock( &teamMut );
            changeState(InWaitForLaunchPad);
        }else if (stan == InLaunchPad){
            
            sleep(SEC_IN_STATE);
            debug("BUUUUM, weeeeeee!!!");
            sleep(SEC_IN_STATE);
            changeState(FinishedLaunchPad);
        }else if (stan == FinishedLaunchPad){
            pkt->data = LAUNCHPAD;
            for (int i=0; i<size;i++){
                if (launchpad[i] == 1){
                    sendPacket(pkt, i, ACK);
                }
            }
            
            pthread_mutex_lock( &teamMut );
            current_pads_taken = size;
            current_tables_taken = combined_team_size - team_size + 1;
            pthread_mutex_unlock( &teamMut );
            pkt->data = DESK;
            for (int i=0; i<size;i++){
                sendPacket(pkt, i, REQUEST);
            }
            	pthread_mutex_lock( &teamMut );
				int my_request = lamport;
				pthread_mutex_unlock( &teamMut );

            changeState(InWaitForTable);
        }else if (stan == InTable){
            sleep(SEC_IN_STATE);
            debug("Ok, więc nie udało się bo....");
            sleep(SEC_IN_STATE);
            changeState(FinishedTable);
        } else if (stan == FinishedTable){
            pkt->data = DESK;
            pkt->tables = team_size;
            for (int i=0; i<size;i++){
                if (table[i] == 1){
                    sendPacket(pkt, i, ACK);
                }
            }
            
            pthread_mutex_lock( &teamMut );
            current_tables_taken = combined_team_size;
            pthread_mutex_unlock( &teamMut );
            pkt->data = DESK;
            for (int i=0; i<size;i++){
                sendPacket(pkt, i, REQUEST);
            }
				pthread_mutex_lock( &teamMut );
				int my_request = lamport;
				pthread_mutex_unlock( &teamMut );

            changeState(InWaitForTables);
        }else{
            sleep(SEC_IN_STATE);
        }
    }

    // while (stan != InFinish) {
    //     int perc = random()%100; 

    //     if (perc<STATE_CHANGE_PROB) {
    //         if (stan==InRun) {
	// 	debug("Zmieniam stan na wysyłanie");
		// changeState( InSend );
		// packet_t *pkt = malloc(sizeof(packet_t));
		// pkt->data = perc;
        //         changeTallow( -perc);
        //         sleep( SEC_IN_STATE); // to nam zasymuluje, że wiadomość trochę leci w kanale
        //                               // bez tego algorytm formalnie błędny za każdym razem dawałby poprawny wynik
		// sendPacket( pkt, (rank+1)%size,TALLOWTRANSPORT);
	// 	changeState( InRun );
	// 	debug("Skończyłem wysyłać");
    //         } else {
    //         }
    //     }
    //     sleep(SEC_IN_STATE);
        
    // }
}
