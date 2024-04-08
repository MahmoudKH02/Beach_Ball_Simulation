#include "headers.h"

void send_ball_teamA(int);
void send_ball_teamB(int);
void end_start_new_round(int);


pid_t pids[MAX_PLAYERS];
pid_t drawer_pid;
int drawer_pipe[2];

int current_round = 0;
bool round_finished = false;


int main(int argc, char *argv[]) {
	int i;
    int fd[MAX_PLAYERS][2];
    char* fifos[MAX_PLAYERS];

    // read game settings
    readFile(argv[1]);
    printf("value: %d, %d\n", MAX_ROUNDS, ROUND_TIME);


    for (i = 0; i < MAX_PLAYERS; i++) {
        fifos[i] = (char*) malloc(sizeof("/tmp/fifo--") + 1);
        strcpy(fifos[i], FIFO_PATH);
        
		if (i < 6) {
            char num[20];
            sprintf(num, "A%d", i);
            strcat(fifos[i], num);
            
		} else {
            char num[20];
            sprintf(num, "B%d", i-6);
            strcat(fifos[i], num);
        }

        // Remove any existing file with the same name
        if (access(fifos[i], F_OK) != -1) {
            if (remove(fifos[i]) == -1) {
                perror("Error removing existing FIFO");
                exit(EXIT_FAILURE);
            }
        }

        if (mkfifo(fifos[i], __S_IFIFO | 0777) == -1){
            perror("Fifo Error\n");
            exit(-1);
        }
	}
    

    for ( i = 0; i < MAX_PLAYERS; i++ ) {
        if ( pipe(fd[i]) < 0 ) {
            // Handle error
            printf("Could not fork child %d\n", i);
            exit(-1 * i);
        }

        pids[i] = fork();

        if ( pids[i] < 0 ) {
            // Handle error
            printf("Could not fork child %d\n", i);
            exit(-10 * i);
        }

        // child process
        if ( pids[i] == 0 ) {
            char pipe_r[20];
            char pipe_w[20];
            char player_i[20];

            sprintf(pipe_r, "%d", fd[i][0]);
            sprintf(pipe_w, "%d", fd[i][1]);
            sprintf(player_i, "%d", i);
            
            if (i == LEAD_A || i == LEAD_B) {
                execlp("./teamLead", "teamLead", pipe_r, pipe_w, player_i, fifos[i], NULL);
                // execlp("./teamLead", "teamLead", pipe_r, pipe_w, player_i, "fifos[i]", NULL);
                perror("Exec Teamlead Failed!!\n");
                exit(SIGQUIT);
            } else {
                execlp("./player", "player", pipe_r, pipe_w, player_i, fifos[i], NULL);
                // execlp("./player", "player", pipe_r, pipe_w, player_i, "fifos[i]", NULL);
                perror("Exec Player Failed!!\n");
                exit(SIGQUIT);
            }
        }
    }

    if ( pipe(drawer_pipe) < 0 ) {
        // Handle error
        printf("Could not fork child %d\n", i);
        exit(-1);
    }

    drawer_pid = fork();

    // drawer process
    if (drawer_pid == 0) {
        char r_pipe[20];
        char w_pipe[20];
        char r_time[20];

        sprintf(r_pipe, "%d", drawer_pipe[0]);
        sprintf(w_pipe, "%d", drawer_pipe[1]);
        sprintf(r_time, "%d", ROUND_TIME);

        execlp("./drawer", "drawer", r_pipe, w_pipe, r_time, NULL);
        perror("Exec Drawer Failed!!\n");
        exit(SIGQUIT);
    }

    // Parent process
    for ( i = 0; i < MAX_PLAYERS; i++ ) {
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
    }

    sleep(3);

    // send ball to team leads
    kill(pids[LEAD_A], SIGUSR2);
    kill(pids[LEAD_B], SIGUSR2);

    int sigs[3] = { SIGUSR1, SIGUSR2, SIGALRM };
    void (*functionArray[])(int) = { send_ball_teamA, send_ball_teamB, end_start_new_round };

    set_signals(sigs, functionArray, 3);

    alarm(ROUND_TIME);

    // starting game
    int teamA_wins = 0;
    int teamB_wins = 0;
    char* winner;

    // parent waiting for events from children
    // <<<<-------------------------------------------------------------------------------->>>>
    do {
        pause();

        // check who won
        if (round_finished) {
            winner = winning_team(fd, teamA_wins, teamB_wins);

            printf("The winner is: %s\n", winner);

            printf("-----------------\n");
            printf("Starting Round: %d\n", current_round + 1);
            fflush(NULL);
            printf("-----------------\n");
            fflush(NULL);
        }

    } while (!game_finished(teamA_wins, teamB_wins, winner));

    // wait for children to terminate
    for ( i = 0; i < MAX_PLAYERS; i++ ) {
        wait(NULL);

        // close the pipes
        close(fd[i][0]);
        close(fd[i][1]);
    }

    int status;
    waitpid(drawer_pid, &status, 0);

    // remove the fifos
    for (i = 0; i < MAX_PLAYERS; i++)
        unlink(fifos[i]);


    for (i = 0; i < MAX_PLAYERS; i++)
        free(fifos[i]);

    return 0;
}


void send_ball_teamA(int sig) {
    kill(pids[LEAD_A], SIGUSR2);
    // signal drawer.
    kill(drawer_pid, SIGUSR1);
}

void send_ball_teamB(int sig) {
    kill(pids[LEAD_B], SIGUSR2);
    // signal drawer 
    kill(drawer_pid, SIGUSR2);
}

// alarm signal
void end_start_new_round(int signum) {
    // reset all children
    for (int i = 0; i < MAX_PLAYERS; i++) {
        kill(pids[i], SIGRTMIN);
    }
    current_round++;
    round_finished = true;
    sleep(2);
}


void readFile(char* filename){
    char line[200];
    char label[50];

    FILE *file;
    file = fopen(filename, "r");

    if (file == NULL){
        perror("The file not exist\n");
        exit(-2);
    }

    char separator[] = "=";

    while(fgets(line, sizeof(line), file) != NULL){

        char* str = strtok(line, separator);
        strncpy(label, str, sizeof(label));
        str = strtok(NULL, separator);

        if (strcmp(label, "ROUNDS") == 0){
            MAX_ROUNDS = atoi(str);
        } else if (strcmp(label, "ROUND_TIME") == 0){
            ROUND_TIME = atoi(str);
        }
    }

    fclose(file);
}


char* winning_team(int fd[][2], int teamA_wins, int teamB_wins) {
    // get winning team
    printf("-----------------\n");
    fflush(NULL);
    printf("Round %d Result: \n\n", current_round);
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
        return "Team B";
    else if (ballsB > ballsA)
        return "Team A";

    return "Tie";
}


bool game_finished(int teamA_wins, int teamB_wins, char* last_round_result) {
    // kill all children
    if ( current_round == MAX_ROUNDS || best_of(current_round, teamA_wins, teamB_wins) ) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            kill(pids[i], SIGINT);
        }
        kill(drawer_pid, SIGINT);

        printf("Team A has %d wins\n", teamA_wins);
        printf("Team B has %d wins\n", teamB_wins);

        if (teamA_wins > teamB_wins) {
            printf("Team A Won\n");
        } else if (teamA_wins < teamB_wins) {
            printf("Team B Won\n");
        } else {
            printf("Tie\n");
        }
        fflush(NULL);
        return true;

    } else if (round_finished) { // start new round
        kill(drawer_pid, SIGRTMIN);
        // write result to drawer
        char msg[BUFSIZ];
        sprintf(msg ,"%s", last_round_result);
        write(drawer_pipe[1], msg, sizeof(msg));

        kill(pids[LEAD_A], SIGUSR2);
        kill(pids[LEAD_B], SIGUSR2);

        alarm(ROUND_TIME);
        // sleep(8);
        round_finished = false;
    }
    return false;
}

bool best_of(int curr_round, int teamA_wins, int teamB_wins) {
    if (teamA_wins == (MAX_ROUNDS / 2 + 1))
        return true;
    else if (teamB_wins == (MAX_ROUNDS / 2 + 1))
        return true;

    return false;
}