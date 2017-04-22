/* BLOTTIERE--MAYO Robin 3200248 
 * CAI Qiaoshan 3605744
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define m 6
#define SIZE_MAX 64  //m=6
#define NB_PAIR 10
#define TAGINIT 0
#define TAGSUCC 1
#define TAGRESP 2


int var_to_find;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


/* ---------- I. Initialisation du Systeme ----------- */
void simulateur(void) {
  int id[NB_PAIR], succ[NB_PAIR], resp[NB_PAIR], finger_rang[NB_PAIR][m],finger_id[NB_PAIR][m], rang_succ;
  int searching_pair, deconnect_pair,deconnect_suc, deconnect_pre;
  int r, s, b,min;
  int i, j, k,tmp;

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

  for(i = 0; i < NB_PAIR; i++) {
    printf("id[%d] : %d,\t succ : %d\tresp : %d\n", i, id[i], succ[i], resp[i]);
  }

  /* calculation de la table finger */
	for(i=0; i<NB_PAIR;i++){      // 
		for(j=0; j< m;j++){
			int val;
			val = (id[i]+pow(2,i))%SIZE_MAX;
			for(k=0;k<NB_PAIR;k++){
				if(resp[k] <= val <= id[k])
					finger_rang[i][j]=k+1;    //numero de rang
					finger_id[i][j]=id[k]; 
				else{
					if(id[NB_PAIR-1] < val)	
						finger_rang[i][j] = 1;
						finger_id[i][j]=id[0];
				}			
			}
		}
	}
  
	min = id[0];

  /* Send to nodes : */
  for(i=1; i<=NB_PAIR; i++) {
    rang_succ = (i%NB_PAIR) + 1;
    MPI_Send(&id[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
    MPI_Send(&succ[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
    MPI_Send(&resp[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
    MPI_Send(&rang_succ, 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
	MPI_Send(&min, 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
  }

  /* ----------- II. Recherche d une donnee : ----------- */
  var_to_find = rand() % SIZE_MAX;
  printf("Key to find = %d\n", var_to_find);
  searching_pair = (rand() % NB_PAIR) + 1;
  printf("Searching pair : %d\n", searching_pair);
  MPI_Send(&var_to_find, 1, MPI_INT, searching_pair, TAGINIT, MPI_COMM_WORLD);





void pair(int rang) {
  int rang_succ, id, succ, resp;
  int key_to_find, deconnect;
	int j,rangnext,idnext;
	bool flag = true;
  MPI_Status status;

  /* Initialisation. */
  MPI_Recv(&id, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  MPI_Recv(&succ, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  MPI_Recv(&resp, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  MPI_Recv(&rang_succ, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  MPI_Recv(&min, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  pthread_mutex_lock(&mutex);
  pthread_mutex_unlock(&mutex);

  MPI_Recv(&key_to_find, 1, MPI_INT, MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
	while(flag){
		for(j = m-1;j>=0;j--){
			rangnext = finger_rang[rang-1][j];
			idnext = finger_id[rang-1][j];
			if(id<key_to_find<idnext){
				if(j==0) {
					flag = false;
					printf("pair %d is responsible of key %d\n",succ,key_to_find); 
				}
			}else{
				break;
			}	
		}
		if(flag)MPI_Send(&key_to_find, 1, MPI_INT, rangnext, TAGINIT, MPI_COMM_WORLD);
	}

}



int main (int argc, char* argv[]) {
  int nb_proc, rang;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

  srand(time(NULL));

  if (nb_proc != NB_PAIR+1) {
    printf("Nombre de processus incorrect !\n");
    MPI_Finalize();
    exit(2);
  }

  MPI_Comm_rank(MPI_COMM_WORLD, &rang);

  if (rang == 0) {
    simulateur();
  } else {
    pair(rang);
  }

  MPI_Finalize();
  return 0;
}
