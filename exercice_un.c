/* BLOTTIERE--MAYO Robin 3200248 
 * CAI Qiaoshan 3605744
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define SIZE_MAX 64
#define NB_PAIR 10
#define TAGINIT 0


int var_to_find;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


/* ---------- I. Initialisation du Systeme ----------- */
void initialization(void) {
  int id[NB_PAIR], succ[NB_PAIR], resp[NB_PAIR], rang_succ;
  int searching_pair;
  int r, s, b;
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

  for(i = 0; i < NB_PAIR; i++) {
    printf("id[%d] : %d,\tsucc : %d\tresp : %d\n", i, id[i], succ[i], resp[i]);
  }

  /* Send to nodes : */
  for(i=1; i<=NB_PAIR; i++) {
    rang_succ = (i%NB_PAIR) + 1;
    MPI_Send(&id[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
    MPI_Send(&succ[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
    MPI_Send(&resp[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
    MPI_Send(&rang_succ, 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
  }

  /* ----------- II. Recherche d une donnee : ----------- */
  var_to_find = rand() % SIZE_MAX;
  printf("Key to find = %d\n", var_to_find);
  searching_pair = (rand() % NB_PAIR) + 1;
  printf("Searching pair : %d\n", searching_pair);
  MPI_Send(&var_to_find, 1, MPI_INT, searching_pair, TAGINIT, MPI_COMM_WORLD);
}

void pair(int rang) {
  int rang_succ, id, succ, resp;
  int key_to_find;
  MPI_Status status;

  /* Initialisation. */
  MPI_Recv(&id, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  MPI_Recv(&succ, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  MPI_Recv(&resp, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  MPI_Recv(&rang_succ, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, NULL);
  pthread_mutex_lock(&mutex);
  pthread_mutex_unlock(&mutex);

  MPI_Recv(&key_to_find, 1, MPI_INT, MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
  if (key_to_find < id && key_to_find > resp) {
    printf("Pair %d is responsible of key %d\n.", rang, key_to_find);
  }
  else {
    printf("Pair %d is sherching kea %d.\n", status.MPI_SOURCE, key_to_find);
    MPI_Send(&key_to_find, 1, MPI_INT, rang_succ, TAGINIT, MPI_COMM_WORLD);
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
    initialization();
  } else {
    pair(rang);
  }

  MPI_Finalize();
  return 0;
}
