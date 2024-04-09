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

struct Teamlead_var {
    int energy;
    int fd_pipe[2];
    char fifo_name[100];
    int player_num;
    int num_balls_team;
    int num_balls_player;
    struct Queue* pass_to_next_team;
};

struct Player_vars {
    int energy;
    char fifo_name[100];
    int player_num;
    int num_balls_player;
};

struct Ball {
    int player_id; // the player that has this ball.
    float colorR;
    float colorG;
    float colorB;
    int target_ball_position; // target position
    float current_ball_position[2]; // current position
};

struct GUIPlayer {
    struct Queue* balls_queue; // index of ball
    bool dropped_ball;
    float energy_bar;
    int num_balls;
};

struct Queue {
    int arr[20];
    int head;
    int tail;
    int max_size;
};

#endif