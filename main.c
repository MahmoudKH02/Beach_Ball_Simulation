#include "headers.h"

void send_ball_teamA(int);
void send_ball_teamB(int);
void end_start_new_round(int);


pid_t pids[NUM_CHILDREN];
int number_of_rounds = 0;

int main(int argc, char *argv[]) {
	int fd[NUM_CHILDREN][2];
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
        char buffer[MSG_SIZE];
        memset(buffer, 0x0, MSG_SIZE);

        switch (i) {
        case LEAD_A:
            sprintf(buffer, "%d,%d", pids[i+1], pids[6]);
            break;

        case LEAD_B:
            sprintf(buffer, "%d,%d", pids[i+1], pids[0]);
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

        close(fd[i][0]); // Close read end
        close(fd[i][1]); // Close write end
    }

    sleep(1);
    // send ball to team leads
    kill(pids[LEAD_A], SIGUSR2);
    kill(pids[LEAD_B], SIGUSR2);
    alarm(50); // 5 minutes

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

void end_start_new_round(int signum){
    for (int i = 0; i < NUM_CHILDREN; i++) {
        kill(pids[i], SIGINT);
    }
}