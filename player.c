#include "headers.h"
#include "functions.h"

void set_signals();
void pass_ball();

void catch_ball(int);
void decrement_energy(int);
void reset(int);

int energy;
int fd_pipe[2];
pid_t next_pid;
int player_num;
int num_balls_player;
bool round_finished;

// struct player_vars vars;


int main(int argc, char *argv[]) {
    if (argc < 4) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    fd_pipe[0] = atoi(argv[1]);
    fd_pipe[1] = atoi(argv[2]);
    
    player_num = atoi(argv[3]);

    next_pid = fetch_next_pid(fd_pipe[0], NULL);
    set_signals();
    init_vars(&energy, &num_balls_player, NULL);

    while (1) {
        alarm(20);
        pause();
        
        round_finished = false;
        unsigned int s = 0;
        // alarm(5);

        while (num_balls_player > 0) {
            if (s == 0)
                s = get_sleep_duration(energy, num_balls_player, player_num);
            
            while ( (s = sleep(s)) > 0 );

            if (num_balls_player > 1)
                s = get_sleep_duration(energy, num_balls_player, player_num); // calculate sleep time for other ball.
            
            if (!round_finished)
                pass_ball();
            // update energy
        }

    }

    // close(fd_pipe[0]);
    // close(fd_pipe[1]); // Close write end

    return 0;
}

// take seed as argument
void set_signals() {
    if ( signal(SIGUSR1, catch_ball) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }

    if ( signal(SIGALRM, decrement_energy) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }

    if ( signal(SIGRTMIN, reset) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }
}


void catch_ball(int sig) {
    num_balls_player++;
}


void pass_ball() {
    kill(next_pid, SIGUSR1);

    if (player_num == 5)
        printf("player (%d) passing ball to (0), E=%d\n", player_num, energy);
    else if (player_num == 11)
        printf("player (%d) passing ball to (6), E=%d\n", player_num, energy);
    else
        printf("player (%d) passing ball to (%d), E=%d\n", player_num, player_num+1, energy);

    fflush(NULL);
    num_balls_player--;
}


void decrement_energy(int sig) {
    if (energy > 50)
        energy -= (rand() % 5) + 1;
}

void reset(int sig) {
    init_vars(&energy, &num_balls_player, NULL);
    round_finished = true;
}