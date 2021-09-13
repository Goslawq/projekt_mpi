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
	         //             pthread_mutex_lock( &teamMut );
               //               my_request = lamport;
                   //           pthread_mutex_unlock( &teamMut );
                changeState( FinishedSetup );
            }
        } else if (stan == FinishedSetup){
	    //debug("my_req przed pierwszym update %i", my_request);
            updateClock(my_request);
	    //debug("my_req po pierwszym %i", my_request);
	    pkt->data = DESK;
	    reqTime = my_request;
	    changeState(InWaitForTables);
            for (int i=0; i<size;i++){
                if(i!=rank)
		    sendPacket(pkt, i, REQUEST);
            }
//				pthread_mutex_lock( &teamMut );
//				my_request = lamport;
//				pthread_mutex_unlock( &teamMut );

       //     changeState(InWaitForTables);
        //} else if (stan == InWaitForTables){
        //    sleep(SEC_IN_STATE);
        } else if (stan == InTables){
            sleep(SEC_IN_STATE);
            debug("Już wiemy co dzisiaj wysadzimy");
            sleep(SEC_IN_STATE);
            changeState(FinishedTables);
        } else if (stan == FinishedTables){
            updateClock(my_request);
	    pkt->data = DESK;
            pkt->tables = team_size;
            for (int i=0; i<size;i++){
                if (table[i] == 1){
          //          debug("finishedDesk loop %i, sending", i);
			sendPacket(pkt, i, ACK);
			table[i] = 0;
                }
            }
            
            pthread_mutex_lock( &teamMut );
            current_tables_taken = combined_team_size;
            current_rooms_taken = size;
            pthread_mutex_unlock( &teamMut );
            pkt->data = ROOM;
	    updateClock(my_request);
	    reqTime = my_request;
	    changeState(InWaitForRoom);
            for (int i=0; i<size;i++){
                if(i!=rank)
		    sendPacket(pkt, i, REQUEST);
            }
//				pthread_mutex_lock( &teamMut );
//				my_request = lamport;
//				pthread_mutex_unlock( &teamMut );
	    debug("Skonczylismy prace przy biurkach");
            //changeState(InWaitForRoom);
        } else if (stan == InRoom){
            sleep(SEC_IN_STATE);
            debug("Z przyjemnością ogłaszamy że tym razem start się uda");
	    debug("---Konferencja zakonczona---");
            sleep(SEC_IN_STATE);
            changeState(FinishedRoom);
        }else if (stan == FinishedRoom){
			//debug("Started FinishedRoom")
            pkt->data = ROOM;
	    updateClock(my_request);
            for (int i=0; i<size;i++){
				//debug("FinishedRoom loop %i", i);
                if (room[i] == 1){
	//			debug("FinishedRoom loop %i, sending packet", i);
                    sendPacket(pkt, i, ACK);
		    room[i] = 0;
                }
            }
			//debug("Przed free");
            
			//debug("Sent FinishedRoom Requests")
            pthread_mutex_lock( &teamMut );
            current_rooms_taken = size;
            current_pads_taken = size;
			//debug("mainthread c_p_t = %i", current_pads_taken);
            pthread_mutex_unlock( &teamMut );
            pkt->data = LAUNCHPAD;
	    updateClock(my_request);
	    reqTime = my_request;
            changeState(InWaitForLaunchPad);
	    for (int i=0; i<size;i++){
		if(i!=rank)
                	sendPacket(pkt, i, REQUEST);
            }
//				pthread_mutex_lock( &teamMut );
//				my_request = lamport;
//				pthread_mutex_unlock( &teamMut );
            //changeState(InWaitForLaunchPad);
        }else if (stan == InLaunchPad){
            
            sleep(SEC_IN_STATE);
            debug("BUUUUM, weeeeeee!!!");
	    debug("Z przykroscia zwalniamy stanowisko startowe rakiet :-(");
            sleep(SEC_IN_STATE);
            changeState(FinishedLaunchPad);
        }else if (stan == FinishedLaunchPad){
            pkt->data = LAUNCHPAD;
	    updateClock(my_request);
       	    for (int i=0; i<size;i++){
                if (launchpad[i] == 1){
        //		debug("FinishedLaunchPad loop %i, sending pckt",i);

	    		sendPacket(pkt, i, ACK);
			launchpad[i] = 0;
                }
            }
            
            pthread_mutex_lock( &teamMut );
            current_pads_taken = size;
            current_tables_taken = combined_team_size - team_size + 1;
            pthread_mutex_unlock( &teamMut );
            pkt->data = DESK;
	    updateClock(my_request);
	    reqTime = my_request;
	    changeState(InWaitForTable);
            for (int i=0; i<size;i++){
		if(i!=rank)
                	sendPacket(pkt, i, REQUEST);
            }
  //          	pthread_mutex_lock( &teamMut );
//				my_request = lamport;
//				pthread_mutex_unlock( &teamMut );

            //changeState(InWaitForTable);
        }else if (stan == InTable){
            sleep(SEC_IN_STATE);
            debug("Ok, więc nie udało się bo....");
	    debug("Skonczylismy raport idziemy pracowac nad nowa rakieta");
            sleep(SEC_IN_STATE);
            changeState(FinishedTable);
        } else if (stan == FinishedTable){
            pkt->data = DESK;
	    updateClock(my_request);
            pkt->tables = team_size;
            for (int i=0; i<size;i++){
                if (table[i] == 1){
                    sendPacket(pkt, i, ACK);
		    table[i] = 0;
                }
            }
            
            pthread_mutex_lock( &teamMut );
            current_tables_taken = combined_team_size;
            pthread_mutex_unlock( &teamMut );
            pkt->data = DESK;
	    updateClock(my_request);
	    reqTime = my_request;
	    changeState(InWaitForTables);
            for (int i=0; i<size;i++){
               if(i!=rank)
		    sendPacket(pkt, i, REQUEST);
            }
//				pthread_mutex_lock( &teamMut );
//				my_request = lamport;
//				pthread_mutex_unlock( &teamMut );

            //changeState(InWaitForTables);
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
