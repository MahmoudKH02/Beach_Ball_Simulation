#include "headers.h"

#define NUM_CHILDREN 12
#define NUM_FIFOS 13
#define FIFO_PATH "/tmp/fifo"


int main() {
	int fd[NUM_CHILDREN][2];
	char fifos[NUM_CHILDREN + 1][100]; // 13
	pid_t pids[NUM_CHILDREN];
	int i;

	// create the fifo pipes's names
	for (i = 0; i < NUM_FIFOS; i++) {
		strcpy(fifos[i], FIFO_PATH);
		char player_num[20];

		if (i < 6) {
			strcat(fifos[i], "A");
			sprintf(player_num, "%d", i);
		} else {
			strcat(fifos[i], "B");
			sprintf(player_num, "%d", i-6);
        }
        strcat(fifos[i], player_num);

        // creating teh fifo
        mkfifo(fifos[i], 0666);
	}

    // add the last fifo pipe (between the two team leads)
    strcpy(fifos[NUM_FIFOS-1], FIFO_PATH);
    strcat(fifos[NUM_FIFOS-1], "AB");

    for (i = 0; i < NUM_FIFOS; i++)
        printf("%s ", fifos[i]);

    printf("\n");

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

            if (i != 0 && i != 6) { // not the team leads
                execlp(
                    "./player", "player",
                    pipe_r, pipe_w, player_i,
                    fifos[i-1], fifos[i],
                    NULL
                );
            } else { // team leads
                int prev_player = (i == 0)? 5:11;

                execlp(
                    "./player", "player",
                    pipe_r, pipe_w, player_i,
                    fifos[prev_player], fifos[i], fifos[NUM_FIFOS-1],
                    NULL
                );
            }
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

    // remove the fifos
    for (i = 0; i < NUM_CHILDREN+1; i++)
        unlink(fifos[i]);

    return 0;
}
