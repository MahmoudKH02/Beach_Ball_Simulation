#include "headers.h"

void pass_ball(int, int);
void reset(int);

void catch_ball_from_player(int);
void catch_ball_from_teamlead (int);
void decrement_energy(int);

struct Teamlead_var leader;

bool round_finished;
int slept_time;

int main(int argc, char *argv[]) {
    if (argc < 5) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    leader.fd_pipe[0] = atoi(argv[1]);
    leader.fd_pipe[1] = atoi(argv[2]);
    
    leader.player_num = atoi(argv[3]);

    strcpy(leader.fifo_name, argv[4]);

    // read the pids (pid of the first player in the team, and the other team lead)
    pid_t other_team_lead;
    pid_t next_pid = fetch_next_pid(leader.fd_pipe[0], &other_team_lead);

    // set the signal handlers
    int sigArr[4] = { SIGUSR1, SIGUSR2, SIGALRM, SIGRTMIN };
    void (*functionArray[])(int) = { catch_ball_from_player, catch_ball_from_teamlead, decrement_energy, reset };
    set_signals(sigArr, functionArray, 4);

    // initialize the variables, and states
    init_vars(&leader.energy, &leader.num_balls_player, &leader.num_balls_team, leader.fifo_name);
    
    leader.pass_to_next_team = NULL;
    leader.pass_to_next_team = create_queue(leader.pass_to_next_team);

    
    // decrement energy every 17 seconds
    alarm(17);

    while (1) {
        pause();

        round_finished = false;
        unsigned int s = 0;

        while (leader.num_balls_player > 0) {
            if (s == 0)
                s = get_sleep_duration(leader.energy, leader.num_balls_player, leader.player_num, leader.fifo_name);
            slept_time = s;
            
            // sleep the required amount
            while ( (s = sleep(s)) > 0 && !round_finished);
            
            if (leader.num_balls_player > 1)
                s = get_sleep_duration(leader.energy, leader.num_balls_player, leader.player_num, leader.fifo_name);
            
            if (round_finished)
                break;

            pass_ball(next_pid, other_team_lead);
        }
    }

    close(leader.fd_pipe[0]);
    close(leader.fd_pipe[1]);
    delete_queue(leader.pass_to_next_team);

    return 0;
}


// catch the ball from a player in the same team (player 5).
// queue is used so that the team lead sends balls in the correct order.
void catch_ball_from_player(int sig) {
    enqueue(leader.pass_to_next_team, true);
    leader.num_balls_player++;
}


// catch ball from the other teamlead (or the parent process)
// if the ball is caught from the other team lead, the the ball should be passed to player 1 of the team.
void catch_ball_from_teamlead(int sig) {
    enqueue(leader.pass_to_next_team, false);
    leader.num_balls_player++;
    leader.num_balls_team++;
    printf("Team Lead (%d) has %d balls in his hand, his team has %d balls\n", leader.player_num, leader.num_balls_player, leader.num_balls_team);
}


// pass the ball to eithter the other team lead or player 1 of the same team.
void pass_ball(int next_pid, int other_team_lead) {
    leader.num_balls_player--;
    char msg_s[BUFSIZ];

    if (leader.energy > 25)
        leader.energy -= (rand() % 2) + 1;

    // the ball should be passed to the other teamlead
    if ( dequeue(leader.pass_to_next_team) ) { // get the first ball that reached the teamlead
        kill(other_team_lead, SIGUSR2);
        printf("teamlead (%d) passing ball to teamlead (%d)--Balls%d, sleep: %d\n",
                leader.player_num, (leader.player_num==LEAD_A)? LEAD_B:LEAD_A, leader.num_balls_team, slept_time);

        fflush(NULL);
        leader.num_balls_team--;

        sprintf(msg_s, "P,%d,%d", (leader.player_num == LEAD_A)? LEAD_B:LEAD_A, leader.energy);

        // ask parent to pass ball, if the team has no balls
        if (leader.num_balls_team == 0) {
            if (leader.player_num == LEAD_A)
                kill(getppid(), SIGUSR1);
            else
                kill(getppid(), SIGUSR2);
        }

    } else { // the ball should be passed to player 1 of the same team
        kill(next_pid, SIGUSR1);

        if (leader.player_num == 5)
            printf("player (%d) passing ball to (0), E=%d, sleep:%d\n", leader.player_num, leader.energy, slept_time);
        else if (leader.player_num == 11)
            printf("player (%d) passing ball to (6), E=%d, sleep:%d\n", leader.player_num, leader.energy, slept_time);
        else
            printf("player (%d) passing ball to (%d), E=%d, sleep:%d\n", leader.player_num, leader.player_num+1, leader.energy, slept_time);

        fflush(NULL);

        sprintf(msg_s, "P,%d,%d", leader.player_num + 1, leader.energy);
    }

    // write the passing of ball in the fifo for the drawer (openGL)
    write_fifo(msg_s, leader.fifo_name);
}


// decrement energy every 17 seconds
// SIGALRM.
void decrement_energy(int sig) {
    srand(time(NULL));

    if (leader.energy > 30)
        leader.energy -= (rand() % 5) + 1;

    alarm(17);
}


// reset all variables and state of the process, and sleep to give the openGL time.
// before the variables are reset, each teamlead sends the number of balls his team has, through the pipe 
// SIGRTMIN.
void reset(int sig) {
    char message[20];
    sprintf(message, "%d", leader.num_balls_team);

    // write the number of balls of team to the parent.
    write(leader.fd_pipe[1], message, sizeof(message));

    init_vars(&leader.energy, &leader.num_balls_player, &leader.num_balls_team, leader.fifo_name);
    clear_queue(leader.pass_to_next_team);

    round_finished = true;

    // sleep to give the openGL time until next round begins
    sleep(3);
}