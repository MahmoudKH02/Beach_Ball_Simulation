#ifndef __CONSTANTS__
#define __CONSTANTS__

#define NUM_CHILDREN 12
#define LEAD_A 0
#define LEAD_B 6
#define LAST_A 5
#define LAST_B 11

// enum TeamPlayers {
//     LEAD_A, P1A, P2A, P3A, P4A, P5A,
//     LEAD_B, P1B, P2B, P3B, P4B, P5B
// };

struct lead_vars {
    int energy;
    int fd_pipe[2];
    int player_num;
    int num_balls_team;
    pid_t next_pid;
    pid_t other_team_lead;
};

struct player_vars {
    int energy;
    int fd_pipe[2];
    int player_num;
    pid_t next_pid;
};

// int RANDOM_NUMBER;
// float THE_PRICE;
// char THE_LEADER_NAME[25];

#endif