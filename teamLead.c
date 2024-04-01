#include "headers.h"

pid_t fetch_next_pid (int, pid_t*);

int main(int argc, char *argv[]) {

    if (argc < 4) {
        perror("Not enough arguments\n");
        exit(-1);
    }

    int pipe[2] = {atoi(argv[1]), atoi(argv[2])};
    int player_num = atoi(argv[3]);
    
    pid_t next_pid;
    pid_t other_team_lead;

    next_pid = fetch_next_pid(pipe[0], &other_team_lead);

    close(pipe[0]);

    char message[BUFFER_SIZE];
    memset(message, 0x0, BUFFER_SIZE);

    sprintf(message, "Iam (%d) --> (%d) | (%d)\n", player_num, getpid(), next_pid, other_team_lead);
    write(pipe[1], message, strlen(message)); // Write to pipe
    close(pipe[1]); // Close write end

  return 0;
}

pid_t fetch_next_pid(int r_pipe, pid_t* other_team_lead) {
    char message[512];

    read(r_pipe, message, sizeof(message));

    char* temp = strtok(message, ",");
    pid_t next_pid = atoi(temp);
    
    temp = strtok('\0', ",");
    *other_team_lead = atoi(temp);

    return next_pid;
}

void start_playing() {
    // ...
}
