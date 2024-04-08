#include "headers.h"

void pass_ball(int, int);
void reset(int);

void catch_ball_from_player(int);
void catch_ball_from_teamlead (int);
void decrement_energy(int);

bool round_finished;

struct Teamlead_var leader;

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

    // read the pids from parent
    pid_t other_team_lead;
    pid_t next_pid = fetch_next_pid(leader.fd_pipe[0], &other_team_lead);

    // set the sensitivity of signals
    int sigArr[4] = { SIGUSR1, SIGUSR2, SIGALRM, SIGRTMIN };
    void (*functionArray[])(int) = { catch_ball_from_player, catch_ball_from_teamlead, decrement_energy, reset };

    set_signals(sigArr, functionArray, 4);

    init_vars(&leader.energy, &leader.num_balls_player, &leader.num_balls_team, leader.fifo_name);
    
    leader.pass_to_next_team = NULL;
    leader.pass_to_next_team = create_queue(leader.pass_to_next_team);


    alarm(17);

    while (1) {
        pause();

        round_finished = false;
        unsigned int s = 0;

        while (leader.num_balls_player > 0) {
            if (s == 0)
                s = get_sleep_duration(leader.energy, leader.num_balls_player, leader.player_num, leader.fifo_name);
            slept_time = s;
            
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


void catch_ball_from_player(int sig) {
    enqueue(leader.pass_to_next_team, true);
    // leader.pass_to_next_team[leader.num_balls_player] = true;
    leader.num_balls_player++;
}


void catch_ball_from_teamlead(int sig) {
    enqueue(leader.pass_to_next_team, false);
    // leader.pass_to_next_team[leader.num_balls_player] = false;
    leader.num_balls_player++;
    leader.num_balls_team++;
    printf("Team Lead (%d) has %d balls in his hand, his team has %d balls\n", leader.player_num, leader.num_balls_player, leader.num_balls_team);
}


void pass_ball(int next_pid, int other_team_lead) {
    leader.num_balls_player--;
    leader.energy -= (rand() % 2) + 1;
    char msg_s[BUFSIZ];

    // if (leader.pass_to_next_team[leader.num_balls_player]) {
    if ( dequeue(leader.pass_to_next_team) ) {
        kill(other_team_lead, SIGUSR2);
        printf("teamlead (%d) passing ball to teamlead (%d)--Balls%d, sleep: %d\n",
                leader.player_num, (leader.player_num==LEAD_A)? LEAD_B:LEAD_A, leader.num_balls_team, slept_time);

        fflush(NULL);
        leader.num_balls_team--;

        sprintf(msg_s, "P,%d,%d", (leader.player_num == LEAD_A)? LEAD_B:LEAD_A, leader.energy);

        // ask parent to pass ball
        if (leader.num_balls_team == 0) {
            if (leader.player_num == LEAD_A)
                kill(getppid(), SIGUSR1);
            else
                kill(getppid(), SIGUSR2);
        }
    } else {
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

    write_fifo(msg_s, leader.fifo_name);

    srand(time(NULL));
}


void decrement_energy(int sig) {
    srand(time(NULL));

    if (leader.energy > 30)
        leader.energy -= (rand() % 5) + 1;

    alarm(17);
}

void reset(int sig) {
    char message[20];
    sprintf(message, "%d", leader.num_balls_team);

    write(leader.fd_pipe[1], message, sizeof(message));

    init_vars(&leader.energy, &leader.num_balls_player, &leader.num_balls_team, leader.fifo_name);

    clear_queue(leader.pass_to_next_team);

    round_finished = true;
    sleep(3);
}