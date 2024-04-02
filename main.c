#include "headers.h"

#define NUM_CHILDREN 12

int main() {
	int fd[NUM_CHILDREN][2];
	pid_t pids[NUM_CHILDREN];
	int i;

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
        char buffer[BUFFER_SIZE];
        memset(buffer, 0x0, BUFFER_SIZE);

        switch (i) {
        case 0:
            sprintf(buffer, "%d,%d", pids[i+1], pids[6]);
            break;

        case 6:
            sprintf(buffer, "%d,%d", pids[i+1], pids[0]);
            break;

        case 5:
            sprintf(buffer, "%d", pids[0]);
            break;

        case 11:
            sprintf(buffer, "%d", pids[6]);
            break;

        default:
            sprintf(buffer, "%d", pids[i+1]);
        }

        write(fd[i][1], buffer, sizeof(buffer));

        sleep(1);

        printf("Child %d pid=%d\n", i, pids[i]);
        fflush(NULL);

        close(fd[i][0]); // Close read end
        close(fd[i][1]); // Close write end
    }

    for ( i = 0; i < NUM_CHILDREN; i++ ) {
        wait(NULL);
    }

    return 0;
}
