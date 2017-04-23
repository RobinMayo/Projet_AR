/* BLOTTIERE--MAYO Robin 3200248
 * CAI Qiaoshan 3605744
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define SIZE_MAX 64  //m=6
#define NB_PAIR 10
#define TAGINIT 100
#define TAGDECONNECT 30
#define TAGRECONNECT 40
#define TAGFIN 50


int var_to_find;


/* ---------- I. Initialisation du Systeme ----------- */
void initialization(void) {
	int id[NB_PAIR], succ[NB_PAIR], resp[NB_PAIR], rang_succ;
	int searching_peer, deconnect_peer, deconnect_suc, deconnect_pre, reconnect_peer;
	int reconnect_value;
	int r, b, skip_message;
	int i, j, tmp;
	
	for(i = 0; i < NB_PAIR; i++) {
		id[i] = SIZE_MAX;
		succ[i] = SIZE_MAX;
		resp[i] = SIZE_MAX;
	}
	/* Initialize id. */
	for(i = 0; i < NB_PAIR; i++) {
		do {
			r = rand() % SIZE_MAX;
			if(id[i] == SIZE_MAX) {
				/* Verify unity. */
				b = 1;
				for(j = 0; j < NB_PAIR+i; j++) {
					if(id[j%NB_PAIR] == r) {
						b = 0;
						break;
					}
				}
				if(b == 1)
					id[i] = r;
			}
		} while(id[i] == SIZE_MAX);
	}
	/* Sort tab : */
	for(i = 0; i < NB_PAIR; i++) {
		tmp = id[i];
		for(j = i+1; j < NB_PAIR; j++) {
			if(id[i] > id[j]){
				tmp = id[j];
				id[j]= id[i];
				id[i] = tmp;
			}
		}
	}
	
	/* Initialization of succ : */
	for(i = 0; i < NB_PAIR; i++) {
		succ[i] = id[(i+1)%NB_PAIR];
	}
	
	/* Initialization of resp : */
	for(i = 1; i < NB_PAIR+1; i++) {
		resp[i%NB_PAIR] = id[(i-1)%NB_PAIR] + 1;
	}
	
	for(i = 0; i < NB_PAIR; i++)
		printf("Rang : %d,\t resp : %d,\t id : %d,\t succ : %d\n", i+1, resp[i], id[i], succ[i]);
	
	/* Send to nodes : */
	for(i=0; i<NB_PAIR; i++) {
		rang_succ = ((i+1)%NB_PAIR) + 1;
		MPI_Send(&id[i], 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
		MPI_Send(&succ[i], 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
		MPI_Send(&resp[i], 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
		MPI_Send(&rang_succ, 1, MPI_INT, i+1, TAGINIT, MPI_COMM_WORLD);
	}
	
	/* ----------- II. Recherche d une donnee : ----------- */
	
	puts("\n***** II. Recherche d une donnee : *****\n");
	var_to_find = rand() % SIZE_MAX;
	printf("Key to find = %d\n", var_to_find);
	searching_peer = (rand() % NB_PAIR) + 1;
	printf("Searching pair : %d\n", searching_peer);
	sleep(1);
	MPI_Send(&var_to_find, 1, MPI_INT, searching_peer, TAGINIT, MPI_COMM_WORLD);
	
	// Wall : to be shure every peer pass step 2.
	for(i = 0; i < NB_PAIR; i++)
		MPI_Recv(&searching_peer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
	
	/* ----------- III. Gestion de la Dynamicite du Systeme : ----------- */
	
	puts("\n***** III. Gestion de la Dynamicite du Systeme : *****\n");
	deconnect_peer = (rand() % NB_PAIR) + 1;    //1--NB_PAIR+1
	deconnect_suc = (deconnect_peer % NB_PAIR) + 1;
	deconnect_pre = (deconnect_peer - 1) ? (deconnect_peer - 1) : NB_PAIR;
	
	//printf("deconnect_peer : %d,\t", deconnect_peer);
	//printf("deconnect_suc : %d,\t deconnect_pre : %d.\n", deconnect_suc, deconnect_pre);
	
	r = resp[deconnect_pre%NB_PAIR];
	
	printf("Peer %d deconnect.\n", deconnect_peer);
	//printf("r : %d\n", r);
	MPI_Send(&r, 1, MPI_INT, deconnect_pre, TAGDECONNECT, MPI_COMM_WORLD);
	
	// Wall : to be shure every peer pass deconection on step 3.
	for(i = 0; i < NB_PAIR; i++)
		MPI_Recv(&searching_peer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
	
	reconnect_peer = deconnect_peer;
	b = 0;
	do {
		b = 1;
		reconnect_value = rand() % SIZE_MAX;
		for(i=0; i<NB_PAIR; i++) {
			if (id[i] == reconnect_value)
				b = 0;
		}
	} while(b != 1);
	printf("reconnect_value = %d.\n", reconnect_value);
	sleep(1);
	MPI_Send(&reconnect_value, 1, MPI_INT, ((deconnect_peer+1)%NB_PAIR)+1,
					 reconnect_peer, MPI_COMM_WORLD);
	
	MPI_Recv(&searching_peer, 1, MPI_INT, MPI_ANY_SOURCE, TAGFIN, MPI_COMM_WORLD, NULL);
	sleep(3);
	printf("last peer before ex-deconnected peer = %d\n", searching_peer);
	puts("***** PRINT *****");
	for(i=1; i<NB_PAIR+1; i++) {
		rang_succ = (i%NB_PAIR)+1;
		if(rang_succ == searching_peer) {
			MPI_Send(&deconnect_peer, 1, MPI_INT, i, TAGFIN, MPI_COMM_WORLD);
			//printf("rang_succ = %d\n", deconnect_peer);
		}
		if(i == deconnect_peer) {
			//rang_succ = (deconnect_peer+1)%NB_PAIR;
			MPI_Send(&searching_peer, 1, MPI_INT, i, TAGFIN, MPI_COMM_WORLD);
		}
		else {
			MPI_Send(&rang_succ, 1, MPI_INT, i, TAGFIN, MPI_COMM_WORLD);
			//printf("rang_succ = %d\n", rang_succ);
		}
		sleep(1);
	}
	puts("Init process (0) shut down.");
}


void pair(int rang) {
	int rang_succ, id, succ, resp;
	int key_to_find, deconnect, message_from;
	int searching_peer = 0;
	MPI_Status status;
	
	/* Initialisation. */
	MPI_Recv(&id, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
	MPI_Recv(&succ, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
	MPI_Recv(&resp, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
	MPI_Recv(&rang_succ, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
	//printf("Peer %d have id %d and rang_succ %d\n", rang, id, rang_succ);
	
	/* II. Recherche d une donnee : */
	MPI_Recv(&key_to_find, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	if(status.MPI_SOURCE == 0)
		searching_peer = rang;
	else
		searching_peer = status.MPI_TAG;
	
	if (key_to_find == -1) {
		//puts("Key was found, step 2 is skipped by this peer.");
		MPI_Send(&key_to_find, 1, MPI_INT, rang_succ, TAGINIT, MPI_COMM_WORLD);
	}
	else {
		if((key_to_find <= id && key_to_find >= resp) || (key_to_find >= resp && id < resp)) {
			printf("Peer %d with id %d is responsible of key %d.\n", rang, id, key_to_find);
			key_to_find = -1;
			MPI_Send(&key_to_find, 1, MPI_INT, rang_succ, TAGINIT, MPI_COMM_WORLD);
		}
		else {
			printf("On peer %d : peer %d search key %d.\n", rang, searching_peer, key_to_find);
			MPI_Send(&key_to_find, 1, MPI_INT, rang_succ, searching_peer, MPI_COMM_WORLD);
		}
	}
	
	// Wall : to be shure every peer pass step 2.
	if(status.MPI_SOURCE == 0)
		MPI_Recv(&key_to_find, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	MPI_Send(&key_to_find, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD);
	//printf("Peer %d is waiting.\n", rang);
	
	/* III. Gestion de la Dynamicite du Systeme : */
	MPI_Recv(&deconnect, 1, MPI_INT, MPI_ANY_SOURCE, TAGDECONNECT, MPI_COMM_WORLD, &status);
	searching_peer = status.MPI_TAG;
	
	// Previous deconected peer.
	if(status.MPI_SOURCE == 0) {
		//printf("Previous deconected peer is : %d, it recive a message from %d.\n", rang, status.MPI_SOURCE);
		MPI_Send(&deconnect, 1, MPI_INT, rang_succ, TAGDECONNECT, MPI_COMM_WORLD);
		// Change the adress of the successor.
		rang_succ = (rang_succ % NB_PAIR) + 1;
		message_from = (rang+2 > NB_PAIR) ? rang-8 : rang+2;
		MPI_Recv(&succ, 1, MPI_INT, message_from, TAGDECONNECT, MPI_COMM_WORLD, &status);
		key_to_find = 100;
	}
	else {
		// Deconected peer.
		if(deconnect == resp) {
			//printf("Deconected peer is : %d.\n", rang);
			id = -1;
			MPI_Send(&deconnect, 1, MPI_INT, rang_succ, TAGDECONNECT, MPI_COMM_WORLD);
			rang_succ = -1;
		}
		else {
			// Succor of deconected peer.
			if(deconnect != -1) {
				resp = deconnect;
				deconnect = -1;
				printf("peer %d : resp is %d, id is %d, succ is %d.\n", rang, resp, id, succ);
				MPI_Send(&deconnect, 1, MPI_INT, rang_succ, TAGDECONNECT, MPI_COMM_WORLD);
				//printf("id = %d,\t (rang-2)%NB_PAIR = %d", id, (rang-2)%NB_PAIR);
				MPI_Send(&id, 1, MPI_INT, (rang-2)%NB_PAIR, TAGDECONNECT, MPI_COMM_WORLD);
			}
			else
				MPI_Send(&deconnect, 1, MPI_INT, rang_succ, TAGDECONNECT, MPI_COMM_WORLD);
		}
	}
	
	// Wall : to be shure every peer pass deconection on step 3.
	if(key_to_find == 100) {
		MPI_Recv(&key_to_find, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		//printf("Peer %d recive a message from %d.\n", rang, status.MPI_SOURCE);
		MPI_Send(&key_to_find, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD);
	}
	else
		MPI_Send(&key_to_find, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD);
	//printf("Peer %d is waiting.\n", rang);
	key_to_find = 100;
	MPI_Recv(&key_to_find, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	searching_peer = status.MPI_TAG;
	//printf("***** Peer %d enter to deconnection step with a message from %d.\n", rang, status.MPI_SOURCE);
	
	if(status.MPI_TAG == TAGRECONNECT) {
		//puts("status.MPI_TAG == TAGRECONNECT");
		id = key_to_find;
		MPI_Recv(&succ, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
		MPI_Recv(&resp, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
		MPI_Recv(&rang_succ, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
		MPI_Send(&rang_succ, 1, MPI_INT, 0, TAGFIN, MPI_COMM_WORLD);
		printf("Peer %d is now reconected with values :\n", rang);
		printf("id = %d,\t succ = %d,\t resp = %d,\t rang_succ = %d.\n", id, succ, resp, rang_succ);
	}
	else {
		if (key_to_find == -1) {
			printf("Message from %d. Peer reconnect, step 3 is skipped by peer %d.\n", status.MPI_SOURCE, rang);
			MPI_Send(&key_to_find, 1, MPI_INT, rang_succ, TAGINIT, MPI_COMM_WORLD);
		}
		else {
			if((key_to_find < id && key_to_find >= resp) ||
				 (key_to_find < id && id < resp) ||
				 (key_to_find >= resp && id < resp)) {
				//printf("rang = %d, key_to_find = %d, id = %d, resp = %d.\n", rang, key_to_find, id, resp);
				// For the peer who wants to be reconnected.
				MPI_Send(&key_to_find, 1, MPI_INT, searching_peer, TAGRECONNECT, MPI_COMM_WORLD);
				MPI_Send(&id, 1, MPI_INT, searching_peer, TAGRECONNECT, MPI_COMM_WORLD);
				MPI_Send(&resp, 1, MPI_INT, searching_peer, TAGRECONNECT, MPI_COMM_WORLD);
				resp = key_to_find +1;
				MPI_Send(&rang, 1, MPI_INT, searching_peer, TAGRECONNECT, MPI_COMM_WORLD);
				// For the successor.
				key_to_find = -1;
				MPI_Send(&key_to_find, 1, MPI_INT, rang_succ, TAGINIT, MPI_COMM_WORLD);
			}
			else {
				printf("Request on peer : %d with id %d\n", rang, id);
				//printf("Message recived from peer with id %d who want to be reconected. ", key_to_find);
				//printf("Its id is %d. Its adress is %d. Rang_succ is %d.\n", id, searching_peer, rang_succ);
				MPI_Send(&key_to_find, 1, MPI_INT, rang_succ, searching_peer, MPI_COMM_WORLD);
			}
		}
	}
	if(status.MPI_SOURCE == 0) {
		sleep(3);
		printf("Peer %d send TAGFIN to peer 1.\n", rang);
		MPI_Recv(&key_to_find, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
	
	//printf("Peer %d is waiting.\n", rang);
	
	/* Final print : */
	MPI_Recv(&rang_succ, 1, MPI_INT, 0, TAGFIN, MPI_COMM_WORLD, &status);
	printf("resp : %d,\t id : %d,\t succ : %d\t\t(rang : %d)\t(rang_succ : %d)\n",
				 resp, id, succ, rang, rang_succ);
	
	//printf("Peer %d shut down.\n", rang);
}



int main (int argc, char* argv[]) {
	int nb_proc, rang;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
	
	srand(time(NULL));
	
	if (nb_proc != NB_PAIR+1) {
		printf("Nombre de processus incorrect ! Must be 10.\n");
		MPI_Finalize();
		exit(2);
	}
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rang);
	
	if (rang == 0) {
		initialization();
	} else {
		pair(rang);
	}
	
	MPI_Finalize();
	return 0;
}
