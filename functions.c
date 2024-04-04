#include "headers.h"
#include "functions.h"


pid_t fetch_next_pid(int r_fd_pipe, int* other_team_lead) {
    char message[BUFSIZ];

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

void init_vars(int* energy, int* num_balls_player, int* num_balls_team) {
    // initialize energy
    srand(getpid());
    *energy = (rand() % 31) + 70; // 70 - 100

    *num_balls_player = 0;

    if (num_balls_team)
        *num_balls_team = 0;
}

// short pause
unsigned int get_sleep_duration(int energy, int balls, int player_num) {
    srand(time(NULL));

    int duration;
    // int duration =  (duration / energy);

    if (energy <= 100 && energy > 90)
        duration = (rand() % 2) + 1;
    else if (energy <= 90 && energy > 80)
        duration = (rand() % 3) + 2;
    else if (energy <= 80 && energy > 70)
        duration = (rand() % 3) + 4;
    else if (energy <= 70 && energy > 60)
        duration = (rand() % 3) + 6;
    else
        duration = (rand() % 3) + 8;

    // calculate drop probability
    int drop_proba;

    // get the drop probability
    if (balls <= 3)
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
