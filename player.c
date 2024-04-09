#include "headers.h"

void pass_ball(int);

void catch_ball(int);
void decrement_energy(int);
void reset(int);


struct Player_vars player;

bool round_finished;
int slept_time;

int main(int argc, char *argv[]) {
    if (argc < 5) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    int fd_pipe[2];
    fd_pipe[0] = atoi(argv[1]);
    fd_pipe[1] = atoi(argv[2]);
    
    player.player_num = atoi(argv[3]);
    strcpy(player.fifo_name, argv[4]);
    
    // read the pid for the next player from the pipe (from parent)
    pid_t next_pid = fetch_next_pid(fd_pipe[0], NULL);

    // set the signal handlers
    int sigArr[3] = { SIGUSR1, SIGALRM, SIGRTMIN };
    void (*functionArray[])(int) = { catch_ball, decrement_energy, reset };
    set_signals(sigArr, functionArray, 3);

    // initialize the variables
    init_vars(&player.energy, &player.num_balls_player, NULL, player.fifo_name);

    // decrement energy every 17 seconds
    alarm(17);

    while (1) {
        pause();
        
        round_finished = false;
        unsigned int s = 0;

        while (player.num_balls_player > 0) {
            // get the random short pause.
            if (s == 0)
                s = get_sleep_duration(player.energy, player.num_balls_player, player.player_num, player.fifo_name);
            slept_time = s;
            
            // sleep the required duration.
            while ( (s = sleep(s)) > 0 && !round_finished );

            if (player.num_balls_player > 1)
                s = get_sleep_duration(player.energy, player.num_balls_player, player.player_num, player.fifo_name); // calculate sleep time for other ball.
            
            if (round_finished)
                break;

            pass_ball(next_pid);
        }
    }

    close(fd_pipe[0]);
    close(fd_pipe[1]);

    return 0;
}


// catch ball from the previous child
// catches SIGUSR1.
void catch_ball(int sig) {
    player.num_balls_player++;
}


// pass ball to next player withing the same team.
void pass_ball(int next_pid) {
    kill(next_pid, SIGUSR1);

    if (player.energy > 25)
        player.energy -= (rand() % 2) + 1; // decrement energy (1-2)

    if (player.player_num == 5)
        printf("player (%d) passing ball to (0), E=%d,sleep:%d\n", player.player_num, player.energy, slept_time);
    else if (player.player_num == 11)
        printf("player (%d) passing ball to (6), E=%d,sleep:%d\n", player.player_num, player.energy, slept_time);
    else
        printf("player (%d) passing ball to (%d), E=%d,sleep:%d\n", player.player_num, player.player_num+1, player.energy, slept_time);

    fflush(NULL);
    player.num_balls_player--;

    char msg_s[BUFSIZ];

    if (player.player_num < 6)
        sprintf(msg_s, "P,%d,%d", (player.player_num + 1) % 6, player.energy);
    else
        sprintf(msg_s, "P,%d,%d", ((player.player_num + 1) % 6) + 6, player.energy);

    // write the passing operation in the fifo, for the drawer (openGL),
    // to update the on screen information.
    write_fifo(msg_s, player.fifo_name);
}


// decrement energy every 17 seconds
// SIGALRM.
void decrement_energy(int sig) {
    srand(time(NULL));

    if (player.energy > 30)
        player.energy -= (rand() % 5) + 1; // (1-5)
    
    alarm(17);
}


// reset all variables and state of the process, and sleep to give the openGL time.
// SIGRTMIN.
void reset(int sig) {
    round_finished = true;
    init_vars(&player.energy, &player.num_balls_player, NULL, player.fifo_name);
    sleep(3);
}