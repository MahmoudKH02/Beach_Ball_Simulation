#ifndef __CONSTANTS__
#define __CONSTANTS__

#define FIFO_PATH "/tmp/fifo"

#define MAX_PLAYERS 12
#define MAX_BALLS 30
#define LEAD_A 0
#define LEAD_B 6
#define LAST_A 5
#define LAST_B 11

static int MAX_ROUNDS;
static int ROUND_TIME;

struct teamlead_var {
    int energy;
    int fd_pipe[2];
    char fifo_name[100];
    int player_num;
    int num_balls_team;
    int num_balls_player;
};

struct player_vars {
    int energy;
    char fifo_name[100];
    int player_num;
    int num_balls_player;
};

struct ball {
    int player_id;
    int target_ball_position; // target position
    float current_ball_position[2]; // current position
};


#endif