#include "headers.h"


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

void set_signals(int signals[], void (*functionArray[])(int), int num_of_signals) {
    for (int i = 0; i < num_of_signals; i++) {
        if ( (signal(signals[i], functionArray[i])) == SIG_ERR ) {
            perror("Sigset can not set SIGQUIT");
            exit(SIGQUIT);
        }
    }
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

// int decrement_energy(int energy) {
//     if (energy > 50)
//         energy -= (rand() % 5) + 1;

//     return energy;
// }

/*
*******************************************************************************

---------------------------| Main Parent Functions |---------------------------

*******************************************************************************
*/

// extern int MAX_ROUNDS;

// char winning_team(int fd[][2], int teamA_wins, int teamB_wins, int current_round) {
//     // get winning team
//     printf("-----------------\n");
//     fflush(NULL);
//     printf("Round %d Result: \n\n", current_round);
//     fflush(NULL);

//     char buffer[BUFSIZ];
//     memset(buffer, 0x0, BUFSIZ);
//     int ballsA, ballsB;
    
//     // team A
//     int nbytes = read(fd[LEAD_A][0], buffer, BUFSIZ-1); // Read up to BUFSIZ-1 bytes

//     if (nbytes >= 0) {
//         buffer[nbytes] = '\0'; // Null-terminate the string
//         printf("Balls A: %s\n", buffer);
//         fflush(NULL);
//         ballsA = atoi(buffer);
//     } else {
//         perror("read from LEAD_A failed");
//     }

//     // Reset buffer before reading next data
//     memset(buffer, 0x0, BUFSIZ);

//     // team B
//     nbytes = read(fd[LEAD_B][0], buffer, BUFSIZ-1); // Read up to BUFSIZ-1 bytes

//     if (nbytes >= 0) {
//         buffer[nbytes] = '\0'; // Null-terminate the string
//         printf("Balls B: %s\n", buffer);
//         fflush(NULL);
//         ballsB = atoi(buffer);
//     } else {
//         perror("read from LEAD_B failed");
//     }

//     if (ballsA > ballsB)
//         return 'B';
//     else if (ballsB > ballsA)
//         return 'A';

//     return '0';
// }


// bool best_of(int curr_round, int teamA_wins, int teamB_wins) {
//     if (teamA_wins == (MAX_ROUNDS / 2 + 1))
//         return true;
//     else if (teamB_wins == (MAX_ROUNDS / 2 + 1))
//         return true;

//     return false;
// }