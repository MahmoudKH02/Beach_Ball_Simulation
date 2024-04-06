#include "headers.h"

void pass_ball(int);

void catch_ball(int);
void decrement_energy(int);
void reset(int);

bool round_finished;

struct player_vars player;

int main(int argc, char *argv[]) {
    if (argc < 4) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    int fd_pipe[2];
    fd_pipe[0] = atoi(argv[1]);
    fd_pipe[1] = atoi(argv[2]);
    
    player.player_num = atoi(argv[3]);

    pid_t next_pid = fetch_next_pid(fd_pipe[0], NULL);

    // set the sensitivity of signals
    int sigArr[3] = { SIGUSR1, SIGALRM, SIGRTMIN };
    void (*functionArray[])(int) = { catch_ball, decrement_energy, reset };

    set_signals(sigArr, functionArray, 3);

    init_vars(&player.energy, &player.num_balls_player, NULL);

    while (1) {
        alarm(20);
        pause();
        
        round_finished = false;
        unsigned int s = 0;

        while (player.num_balls_player > 0) {
            if (s == 0)
                s = get_sleep_duration(player.energy, player.num_balls_player, player.player_num);
            
            while ( (s = sleep(s)) > 0 );

            if (player.num_balls_player > 1)
                s = get_sleep_duration(player.energy, player.num_balls_player, player.player_num); // calculate sleep time for other ball.
            
            if (round_finished)
                break;

            pass_ball(next_pid);
            // update energy
        }
    }

    close(fd_pipe[0]);
    close(fd_pipe[1]);

    return 0;
}


void catch_ball(int sig) {
    player.num_balls_player++;
}


void pass_ball(int next_pid) {
    kill(next_pid, SIGUSR1);

    if (player.player_num == 5)
        printf("player (%d) passing ball to (0), E=%d\n", player.player_num, player.energy);
    else if (player.player_num == 11)
        printf("player (%d) passing ball to (6), E=%d\n", player.player_num, player.energy);
    else
        printf("player (%d) passing ball to (%d), E=%d\n", player.player_num, player.player_num+1, player.energy);

    fflush(NULL);
    player.num_balls_player--;
}


void decrement_energy(int sig) {
    if (player.energy > 50)
        player.energy -= (rand() % 5) + 1;
}

void reset(int sig) {
    round_finished = true;
    init_vars(&player.energy, &player.num_balls_player, NULL);
}