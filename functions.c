#include "headers.h"

void initialize(sigset_t *sigset, bool team_lead) {
    if (team_lead) {
        if ( signal(SIGUSR2, catch_ball_from_teamlead) == SIG_ERR ) {
            perror("Sigset can not set SIGQUIT");
            exit(SIGQUIT);
        }
    }

    if ( signal(SIGUSR1, catch_ball_from_player) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }

    if ( signal(SIGALRM, decrement_energy) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }

    // the sigset to block signals when needed
    sigemptyset(sigset);
    sigaddset(sigset, SIGUSR1);
    sigaddset(sigset, SIGUSR2);
}


void init_variables() {
    // initialize energy
    srand(getpid());
    energy = (rand() % 31) + 70; // 70 - 100

    num_balls_player = 0;
}


pid_t fetch_next_pid(int r_fd_pipe, int* other_team_lead) {
    char message[MSG_SIZE];

    read(r_fd_pipe, message, sizeof(message));

    if (other_team_lead) {
        char* temp = strtok(message, ",");
        int next_pid = atoi(temp);
        
        temp = strtok('\0', ",");
        *other_team_lead = atoi(temp);

        return next_pid;
    }

    return atoi(message);
}

// short pause
unsigned int get_sleep_duration(int energy, int balls, int player_num) {
    srand(time(NULL));

    int duration;
    // int duration =  (duration / energy);

    if (energy <= 100 && energy > 90)
        duration = (rand() % 4) + 1;
    else if (energy <= 90 && energy > 80)
        duration = (rand() % 4) + 4;
    else if (energy <= 80 && energy > 70)
        duration = (rand() % 3) + 7;
    else if (energy <= 70 && energy > 60)
        duration = (rand() % 4) + 9;
    else
        duration = (rand() % 3) + 12;

    int drop_proba;

    // get the drop probability
    if (balls != 0 && balls <= 3)
        drop_proba = (int) 100 - (energy / balls);
    else
        drop_proba = 100;

    bool drop_ball = (rand() % 101) < drop_proba;

    if (drop_ball) {
        printf("player(%d) dropped the ball\n", player_num);
        fflush(NULL);
        duration += 2;
    }

    return duration;
}