#include "headers.h"
#include "functions.h"

void set_signals();
void pass_ball();
void reset(int);

void catch_ball_from_player(int);
void catch_ball_from_teamlead (int);
void decrement_energy(int);

int energy;
int fd_pipe[2];
pid_t next_pid;
pid_t other_team_lead;
int player_num;
int num_balls_team;
int num_balls_player;
bool pass_to_next_team;
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

    next_pid = fetch_next_pid(fd_pipe[0], &other_team_lead);
    set_signals();
    init_vars(&energy, &num_balls_player, &num_balls_team);

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
                s = get_sleep_duration(energy, num_balls_player, player_num);
            
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
    if ( signal(SIGUSR2, catch_ball_from_teamlead) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }

    if ( signal(SIGUSR1, catch_ball_from_player) == SIG_ERR ) {
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


void catch_ball_from_player(int sig) {
    num_balls_player++;
    pass_to_next_team = true;
}


void catch_ball_from_teamlead(int sig) {
    num_balls_player++;
    num_balls_team++;
    pass_to_next_team = false;
    printf("Team Lead (%d) has %d balls in his hand, his team has %d balls\n", player_num, num_balls_player, num_balls_team);
    fflush(NULL);
}


void pass_ball() {
    if (pass_to_next_team) {
        kill(other_team_lead, SIGUSR2);
        printf("teamlead (%d) passing ball to teamlead (%d)--Balls%d\n",
                player_num, (player_num==LEAD_A)? LEAD_B:LEAD_A, num_balls_team);

        fflush(NULL);
        num_balls_team--;
        num_balls_player--;
        pass_to_next_team = false;

        // ask parent to pass ball
        if (num_balls_team == 0) {
            if (player_num == LEAD_A)
                kill(getppid(), SIGUSR1);
            else
                kill(getppid(), SIGUSR2);
        }
        if (num_balls_team == 0) {
            if (player_num == LEAD_A)
                kill(getppid(), SIGUSR1);
            else
                kill(getppid(), SIGUSR2);
        }
        
    } else {
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
}


void decrement_energy(int sig) {
    if (energy > 50)
        energy -= (rand() % 5) + 1;
}

void reset(int sig) {
    char message[20];
    sprintf(message, "%d", num_balls_team);

    write(fd_pipe[1], message, sizeof(message));

    init_vars(&energy, &num_balls_player, &num_balls_team);
    round_finished = true;
}