#ifndef __CONSTANTS__
#define __CONSTANTS__

#define MAX_PLAYERS 12
#define LEAD_A 0
#define LEAD_B 6
#define LAST_A 5
#define LAST_B 11

static int MAX_ROUNDS;
static int ROUND_TIME;


struct teamlead_var {
    int energy;
    int fd_pipe[2];
    int player_num;
    int num_balls_team;
    int num_balls_player;
};

struct player_vars {
    int energy;
    int player_num;
    int num_balls_player;
};


#endif