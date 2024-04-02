#include "headers.h"

pid_t fetch_next_pid (int, int*);
unsigned int get_sleep_duration();
void initialize();

void catch_ball_from_player(int);
void catch_ball_from_teamlead (int sig);

int energy;
int fd_pipe[2];
pid_t next_pid;
pid_t other_team_lead;
int player_num;
int num_balls_team;


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
        // printf("energy: %d\t", energy);
        // fflush(NULL);
        // decreament energy.
        pause();
    }

    // close(fd_pipe[0]);
    // close(fd_pipe[1]); // Close write end

    return 0;
}


void initialize() {
    if (player_num == 6 || player_num == 0) {
        next_pid = fetch_next_pid(fd_pipe[0], &other_team_lead);

        if ( sigset(SIGUSR2, catch_ball_from_teamlead) == SIG_ERR ) {
            perror("Sigset can not set SIGQUIT");
            exit(SIGQUIT);
        }
    }
    else
        next_pid = fetch_next_pid(fd_pipe[0], NULL);

    if ( sigset(SIGUSR1, catch_ball_from_player) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }

    // initialize energy
    srand(getpid());
    energy = (rand() % 51) + 50; // 50 - 100
}


pid_t fetch_next_pid(int r_fd_pipe, int* other_team_lead) {
    char message[512];

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
    // unsigned int duration = get_sleep_duration();
    sleep(1);

    // critical section
    // block signals
    // update energy level
    // sighold(SIGUSR1);
    // sighold(SIGUSR2);

    // if (energy > 10)
    //     energy--;
    
    // sigrelse(SIGUSR1);
    // sigrelse(SIGUSR2);


    if (player_num == 0 || player_num == 6) {
        kill(other_team_lead, SIGUSR2);
        num_balls_team -= 1;

        printf(
            "team lead (%d) passing ball to: (%d) %d--Balls=%d\n",
            player_num, (player_num==6)? 0:6, other_team_lead, num_balls_team
        );
        fflush(NULL);

        if (num_balls_team == 0) {
            if (player_num == 0)
                kill(getppid(), SIGUSR1);
            else if (player_num == 6)
                kill(getppid(), SIGUSR2);
        }
        
    } else {
        // signal next child
        kill(next_pid, SIGUSR1);

        if (player_num == 5)
            printf("player (%d) passing ball to (0), pid:%d\n", player_num, next_pid);
        else if (player_num == 11)
            printf("player (%d) passing ball to (6), pid:%d\n", player_num, next_pid);
        else
            printf("player (%d) passing ball to (%d), pid:%d\n", player_num, player_num+1, next_pid);

        fflush(NULL);
    }
}


void catch_ball_from_teamlead(int sig) {
    // unsigned int duration = get_sleep_duration();

    if (num_balls_team == 0) {
        num_balls_team += 1;
        sleep(1);


        printf(
            "Parent passing ball to (%d), pid:%d--Balls=%d\n",
            player_num, getpid(), num_balls_team
        );
        
        kill(next_pid, SIGUSR1);

    } else {
        num_balls_team += 1;
        sleep(1);
        
        
        printf(
            "teamlead (%d) passing ball to (%d), pid:%d--Balls=%d\n",
            player_num, player_num+1, next_pid, num_balls_team
        );

        kill(next_pid, SIGUSR1);
    }


    // sighold(SIGUSR1);
    // sighold(SIGUSR2);

    // if (energy > 10)
    //     energy--;
    
    fflush(NULL);
    // sigrelse(SIGUSR1);
    // sigrelse(SIGUSR2);
}


unsigned int get_sleep_duration() {
    srand(time(NULL));

    int random = (rand() % 10) + 1;
    float duration = (float) ((100 + random) / energy);

    printf("child (%d) sleep duration: %f\n", player_num, duration);
    fflush(NULL);

    return (unsigned int) (duration);
}
