
#ifndef __HEADER__
#define __HEADER__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/sem.h>
#include <pthread.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdbool.h>
#include <math.h>
#include <GL/glut.h>


#include "constants.h"

unsigned int get_sleep_duration(int energy, int balls, int player_num, char* fifo_name);
pid_t fetch_next_pid(int r_fd_pipe, int* other_team_lead);
void init_vars(int* energy, int* num_balls_player, int* num_balls_team, char* fifo_name);
void set_signals(int signals[], void (*functionArray[])(int), int num_of_signals);
void write_fifo(char* msg, char* fifo_name);
// int decrement_energy(int energy);


/*
*************************** Main Parent Function Headers ***************************
*/

char* winning_team(int fd[][2], int teamA_wins, int teamB_wins);
bool best_of(int curr_round, int teamA_wins, int teamB_wins);
bool game_finished(int teamA_wins, int teamB_wins, char* last_round_result);
void readFile(char* filename);

#endif

