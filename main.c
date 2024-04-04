#include "headers.h"

#define MAX_ROUNDS 3

void send_ball_teamA(int);
void send_ball_teamB(int);
void end_start_new_round(int);

pid_t pids[NUM_CHILDREN];
int fd[NUM_CHILDREN][2];

int current_round = 0;
int teamA_wins = 0;
int teamB_wins = 0;

int main(int argc, char *argv[]) {
	int i;

    signal(SIGALRM, end_start_new_round);

    for ( i = 0; i < NUM_CHILDREN; i++ ) {
        if ( pipe(fd[i]) < 0 ) {
            // Handle error
            printf("Could not fork child %d\n", i);
            exit(-1 * i);
        }

        //fcntl(fd[i], 
        pids[i] = fork();

        if ( pids[i] < 0 ) {
            // Handle error
            printf("Could not fork child %d\n", i);
            exit(-10 * i);
        }

        if ( pids[i] == 0 ) {
            // Child process
            char pipe_r[20];
            char pipe_w[20];
            char player_i[20];

            sprintf(pipe_r, "%d", fd[i][0]);
            sprintf(pipe_w, "%d", fd[i][1]);
            sprintf(player_i, "%d", i);
            
            if (i == LEAD_A || i == LEAD_B) {
                execlp(
                    "./teamLead", "teamLead",
                    pipe_r, pipe_w, player_i,
                    NULL
                );
            }
            execlp(
                "./player", "player",
                pipe_r, pipe_w, player_i,
                NULL
            );
        }
    }

    // Parent process
    for ( i = 0; i < NUM_CHILDREN; i++ ) {
        // write required pid for child
        char buffer[BUFSIZ];
        memset(buffer, 0x0, BUFSIZ);

        switch (i) {
        case LEAD_A:
            sprintf(buffer, "%d,%d", pids[i+1], pids[LEAD_B]);
            break;

        case LEAD_B:
            sprintf(buffer, "%d,%d", pids[i+1], pids[LEAD_A]);
            break;

        case LAST_A:
            sprintf(buffer, "%d", pids[LEAD_A]);
            break;

        case LAST_B:
            sprintf(buffer, "%d", pids[LEAD_B]);
            break;

        default:
            sprintf(buffer, "%d", pids[i+1]);
        }

        write(fd[i][1], buffer, sizeof(buffer));

        printf("Child %d pid=%d\n", i, pids[i]);
        fflush(NULL);
    }

    sleep(1);
    // send ball to team leads
    kill(pids[LEAD_A], SIGUSR2);
    kill(pids[LEAD_B], SIGUSR2);
    alarm(60); // 2 minutes

    if ( signal(SIGUSR1, send_ball_teamA) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }

    if ( signal(SIGUSR2, send_ball_teamB) == SIG_ERR ) {
        perror("Sigset can not set SIGQUIT");
        exit(SIGQUIT);
    }

    for ( i = 0; i < NUM_CHILDREN; i++ ) {
        wait(NULL);
    }

    return 0;
}

void send_ball_teamA(int sig) {
    kill(pids[LEAD_A], SIGUSR2);
}

void send_ball_teamB(int sig) {
    kill(pids[LEAD_B], SIGUSR2);
}

// alarm signal
void end_start_new_round(int signum){
    // reset all children
    for (int i = 0; i < NUM_CHILDREN; i++) {
        kill(pids[i], SIGRTMIN);
    }
    current_round++;

    printf("Starting New Round, Round: %d\n", current_round);
    fflush(NULL);

    char buffer[BUFSIZ];
    memset(buffer, 0x0, BUFSIZ);
    int ballsA, ballsB;
    
    // team A
    int nbytes = read(fd[LEAD_A][0], buffer, BUFSIZ-1); // Read up to BUFSIZ-1 bytes

    if (nbytes >= 0) {
        buffer[nbytes] = '\0'; // Null-terminate the string
        printf("Balls A: %s\n", buffer);
        fflush(NULL);
        ballsA = atoi(buffer);
    } else {
        perror("read from LEAD_A failed");
    }

    // Reset buffer before reading next data
    memset(buffer, 0x0, BUFSIZ);

    // team B
    nbytes = read(fd[LEAD_B][0], buffer, BUFSIZ-1); // Read up to BUFSIZ-1 bytes

    if (nbytes >= 0) {
        buffer[nbytes] = '\0'; // Null-terminate the string
        printf("Balls B: %s\n", buffer);
        fflush(NULL);
        ballsB = atoi(buffer);
    } else {
        perror("read from LEAD_B failed");
    }

    if (ballsA > ballsB)
        teamB_wins++;
    else if (ballsB > ballsA)
        teamA_wins++;
    else {
        printf("Tie Round\n");
        fflush(NULL);
    }

    // kill all children
    if (current_round == MAX_ROUNDS) {
        for (int i = 0; i < NUM_CHILDREN; i++) {
            kill(pids[i], SIGINT);
        }

        printf("Team A has %d wins\n", teamA_wins);
        fflush(NULL);
        printf("Team B has %d wins\n", teamB_wins);
        fflush(NULL);

        if (teamA_wins > teamB_wins) {
            printf("Team A Won\n");
            fflush(NULL);
        }
        else if (teamA_wins < teamB_wins) {
            printf("Team B Won\n");
            fflush(NULL);
        }
        else {
            printf("Tie\n");
            fflush(NULL);
        }
    } else {// start new round
        alarm(60);
        kill(pids[LEAD_A], SIGUSR2);
        kill(pids[LEAD_B], SIGUSR2);
    }
}