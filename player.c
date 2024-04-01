#include "headers.h"

pid_t fetch_next_pid (int, int*);

int main(int argc, char *argv[]) {

    if (argc < 4) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    int pipe[2] = {atoi(argv[1]), atoi(argv[2])};
    int player_num = atoi(argv[3]);
    
    pid_t next_pid;
    pid_t other_team_lead;

    if (player_num == 6 || player_num == 0)
        next_pid = fetch_next_pid(pipe[0], &other_team_lead);
    else
        next_pid = fetch_next_pid(pipe[0], NULL);

    close(pipe[0]);

    char message[BUFFER_SIZE];
    memset(message, 0x0, BUFFER_SIZE);

    sprintf(message, "Iam Child %d pid(%d) --> next(%d) | otherTeam(%d)\n", player_num, getpid(), next_pid, other_team_lead);
    write(pipe[1], message, strlen(message)); // Write to pipe
    close(pipe[1]); // Close write end

  return 0;
}

pid_t fetch_next_pid(int r_pipe, int* other_team_lead) {
    char message[512];

    read(r_pipe, message, sizeof(message));

    if (other_team_lead) {
        char* temp = strtok(message, ",");
        int next_pid = atoi(temp);
        
        temp = strtok('\0', ",");
        *other_team_lead = atoi(temp);

        return next_pid;
    }

    return atoi(message);
}

void start_playing(bool team_leader) {
    // ...
}
