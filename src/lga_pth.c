// Ajuda ao importar pthread_barrier_x:
// https://stackoverflow.com/questions/61647896/unknown-type-name-pthread-barrier-t
#define _POSIX_C_SOURCE 200112L
#include "lga_base.h"
#include "lga_pth.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

typedef struct Thread_Info {
    int k;
    int grid_size;
    int num_threads;
    byte * grid_in;
    byte * grid_out;
    pthread_barrier_t * barr;
} ThreadInfo;

byte get_next_cell(int i, int j, byte *grid_in, int grid_size) {
    byte next_cell = EMPTY;

    for (int dir = 0; dir < NUM_DIRECTIONS; dir++) {
        int rev_dir = (dir + NUM_DIRECTIONS/2) % NUM_DIRECTIONS;
        byte rev_dir_mask = 0x01 << rev_dir;

        int di = directions[i%2][dir][0];
        int dj = directions[i%2][dir][1];
        int n_i = i + di;
        int n_j = j + dj;

        if (inbounds(n_i, n_j, grid_size)) {
            if (grid_in[ind2d(n_i,n_j)] == WALL) {
                next_cell |= from_wall_collision(i, j, grid_in, grid_size, dir);
            }
            else if (grid_in[ind2d(n_i, n_j)] & rev_dir_mask) {
                next_cell |= rev_dir_mask;
            }
        }
    }

    return check_particles_collision(next_cell);
}

void * iter_every_k_cells(void * info){

    // printf("Uma pthread vai começar\n");

    int k,grid_size, num_threads;
    byte * grid_in;
    byte * grid_out;
    ThreadInfo * thread_info;

    int i, j;

    thread_info = (ThreadInfo *) info;

    k = thread_info->k;
    grid_size = thread_info->grid_size;
    num_threads = thread_info->num_threads;
    grid_in = thread_info->grid_in;
    grid_out = thread_info->grid_out;

    // printf("Estou em uma thread\n");

    for(int iter = k; iter < grid_size*grid_size; iter += num_threads){
        j = iter % grid_size;
        i = (iter - j)/grid_size;
        // printf("k: %d i: %d j:%d \n",k,i,j);
        if (grid_in[ind2d(i,j)] == WALL)
            grid_out[ind2d(i,j)] = WALL;
        else
            grid_out[ind2d(i,j)] = get_next_cell(i, j, grid_in, grid_size);
    }

    // printf("Uma pthread vai esperar\n");
    pthread_barrier_wait(thread_info->barr);
    pthread_exit(NULL);

}

static void update(byte *grid_in, byte *grid_out, int grid_size, int num_threads) {

    pthread_barrier_t barreira;
    pthread_t threads[num_threads];
    ThreadInfo infos[num_threads];
    int thread_rc;

    // printf("Thread principal vai iniciar\n");

    pthread_barrier_init(&barreira, (void *) NULL, num_threads+1);

    // printf("Thread principal iniciou\n");

    for(int k = 0; k < num_threads; k += 1){
        infos[k].k = k;
        infos[k].grid_in = grid_in;
        infos[k].grid_out = grid_out;
        infos[k].grid_size = grid_size;
        infos[k].num_threads = num_threads;
        infos[k].barr = &barreira;

        thread_rc = pthread_create(&threads[k], NULL, iter_every_k_cells, (void *) (&infos[k]));

        if(thread_rc) {
            printf("ERRO; Return code de pthread_create() foi %d\n", thread_rc);
            exit(-1);
        }
    }

    // printf("Thread principal vai esperar\n");

    pthread_barrier_wait(&barreira);
    pthread_barrier_destroy(&barreira);
}

void simulate_pth(byte *grid_1, byte *grid_2, int grid_size, int num_threads) {
    printf("Agora vamo começar ao menos\n"); 
    for (int i = 0; i < ITERATIONS/2; i++) {
        update(grid_1, grid_2, grid_size, num_threads);
        update(grid_2, grid_1, grid_size, num_threads);
    }

}
