#include "headers.h"

pid_t fetch_next_pid (int, int*);
unsigned int get_sleep_duration();
void initialize();
bool is_teamlead(int);
void pass_ball();

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

// struct player_vars vars;


int main(int argc, char *argv[]) {
    if (argc < 4) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    fd_pipe[0] = atoi(argv[1]);
    fd_pipe[1] = atoi(argv[2]);
    
    player_num = atoi(argv[3]);

    initialize();

    while (1) {
        alarm(20);
        pause();

        unsigned int s = 0;
        // alarm(5);

        while (num_balls_player > 0) {
            if (s == 0)
                s = get_sleep_duration();
            
            while ( (s = sleep(s)) > 0 );
                // printf("Child (%d) Going Back to sleep for %d\n", player_num, s);
            s = get_sleep_duration();
            pass_ball();
            // update energy
            // energy -= 2;
        }
    }

    // close(fd_pipe[0]);
    // close(fd_pipe[1]); // Close write end

    return 0;
}

// take seed as argument
void initialize() {
    if (is_teamlead(player_num)) {
        next_pid = fetch_next_pid(fd_pipe[0], &other_team_lead);

        if ( signal(SIGUSR2, catch_ball_from_teamlead) == SIG_ERR ) {
            perror("Sigset can not set SIGQUIT");
            exit(SIGQUIT);
        }
    }
    else
        next_pid = fetch_next_pid(fd_pipe[0], NULL);

    if ( signal(SIGUSR1, catch_ball_from_player) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }

    if ( signal(SIGALRM, decrement_energy) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }   

    // initialize energy
    srand(getpid());
    energy = (rand() % 31) + 70; // 70 - 100

    num_balls_player = 0;
    num_balls_team = 0;
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


void catch_ball_from_player(int sig) {
    num_balls_player++;
    pass_to_next_team = true;

    // unsigned int duration = get_sleep_duration();
    
    // sleep(duration);

    // // critical section
    // // block signals
    // // update energy level
    // // sighold(SIGUSR1);
    // // sighold(SIGUSR2);

    // if (energy > 50)
    //     energy -= (rand() % 2) + 1; // 1-2
    
    // // sigrelse(SIGUSR1);
    // // sigrelse(SIGUSR2);


    // if (is_teamlead(player_num)) {
    //     kill(other_team_lead, SIGUSR2);
    //     num_balls_team -= 1;

    //     printf(
    //         "team lead (%d) passing ball to: (%d) %d--Balls=%d, E=%d, sleep:%d\n",
    //         player_num, (player_num==6)? 0:6, other_team_lead, num_balls_team, energy, duration
    //     );
    //     fflush(NULL);

    //     if (num_balls_team == 0) {
    //         if (player_num == 0)
    //             kill(getppid(), SIGUSR1);
    //         else if (player_num == 6)
    //             kill(getppid(), SIGUSR2);
    //     }
        
    // } else {
    //     // signal next child
    //     kill(next_pid, SIGUSR1);

    //     if (player_num == 5)
    //         printf("player (%d) passing ball to (0), pid:%d, E=%d, sleep:%d\n", player_num, next_pid, energy, duration);
    //     else if (player_num == 11)
    //         printf("player (%d) passing ball to (6), pid:%d, E=%d, sleep:%d\n", player_num, next_pid, energy, duration);
    //     else
    //         printf("player (%d) passing ball to (%d), pid:%d, E=%d, sleep:%d\n", player_num, player_num+1, next_pid, energy, duration);

    //     fflush(NULL);
    // }
    // num_balls_player--;
}


void catch_ball_from_teamlead(int sig) {
    num_balls_player++;
    num_balls_team++;
    pass_to_next_team = false;
    printf("Team Lead (%d) has %d balls in his hand, his team has %d balls\n", player_num, num_balls_player, num_balls_team);
    fflush(NULL);

    // unsigned int duration = get_sleep_duration();

    // if (num_balls_team == 0) {
    //     num_balls_team += 1;
    //     sleep(duration);


    //     printf(
    //         "Parent passing ball to (%d), pid:%d--Balls=%d\n",
    //         player_num, getpid(), num_balls_team
    //     );
        
    //     fflush(NULL);
    //     kill(next_pid, SIGUSR1);

    // } else {
    //     num_balls_team += 1;
    //     sleep(duration);
        
        
    //     printf(
    //         "teamlead (%d) passing ball to (%d), pid:%d--Balls=%d\n",
    //         player_num, player_num+1, next_pid, num_balls_team
    //     );

    //     fflush(NULL);
    //     kill(next_pid, SIGUSR1);
    // }

    // if (energy > 50)
    //     energy -= (rand() % 2) + 1; // 1-2


    // // sighold(SIGUSR1);
    // // sighold(SIGUSR2);

    // // if (energy > 10)
    // //     energy--;
    
    // // sigrelse(SIGUSR1);
    // // sigrelse(SIGUSR2);
    // num_balls_player--;
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
        
    } else {
        kill(next_pid, SIGUSR1);

        if (player_num == 5)
            printf("player (%d) passing ball to (0), E=%d--Balls%d\n", player_num, energy, num_balls_player);
        else if (player_num == 11)
            printf("player (%d) passing ball to (6), E=%d--Balls%d\n", player_num, energy, num_balls_player);
        else
            printf("player (%d) passing ball to (%d), E=%d--Balls%d\n", player_num, player_num+1, energy, num_balls_player);

        fflush(NULL);
        num_balls_player--;
    }

    if (num_balls_team == 0) {
        if (player_num == LEAD_A)
            kill(getppid(), SIGUSR1);
        else
            kill(getppid(), SIGUSR2);
    }
}


// short pause
unsigned int get_sleep_duration() {
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
    if (num_balls_player <= 2)
        drop_proba = (int) 100 - (energy / num_balls_player);
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


void decrement_energy(int sig) {
    if (energy > 50)
        energy -= (rand() % 5) + 1;
}


bool is_teamlead(int player_num) {
    return player_num == LEAD_A || player_num == LEAD_B;
}