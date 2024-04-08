#include "headers.h"


pid_t fetch_next_pid(int r_fd_pipe, int* other_team_lead) {
    char message[BUFSIZ];

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

void set_signals(int signals[], void (*functionArray[])(int), int num_of_signals) {
    for (int i = 0; i < num_of_signals; i++) {
        if ( (signal(signals[i], functionArray[i])) == SIG_ERR ) {
            perror("Sigset can not set SIGQUIT");
            exit(SIGQUIT);
        }
    }
}

void init_vars(int* energy, int* num_balls_player, int* num_balls_team, char* fifo_name) {
    // initialize energy
    srand(time(NULL) + getpid());
    *energy = (rand() % 31) + 70; // 70 - 100

    *num_balls_player = 0;

    if (num_balls_team)
        *num_balls_team = 0;
    
    char msg[BUFSIZ];
    sprintf(msg, "E,%d", *energy);
    write_fifo(msg, fifo_name);
}

// short pause
unsigned int get_sleep_duration(int energy, int balls, int player_num, char* fifo_name) {
    srand(time(NULL));

    int duration;

    if (energy <= 100 && energy > 90)
        duration = (rand() % 3) + 3; // 3-5
    else if (energy <= 90 && energy > 80)
        duration = (rand() % 3) + 5; // 5-7
    else if (energy <= 80 && energy > 70)
        duration = (rand() % 3) + 7; // 7-9
    else if (energy <= 70 && energy > 60)
        duration = (rand() % 3) + 9; // 9-11
    else
        duration = (rand() % 3) + 11; // 11-13

    // calculate drop probability
    int drop_proba;

    // get the drop probability
    if (balls <= 3)
        drop_proba = (int) 100 - (energy / balls);
    else    
        drop_proba = 100;

    bool drop_ball = (rand() % 101) < drop_proba;

    if (drop_ball) {
        printf("player(%d) dropped the ball\n", player_num);
        fflush(NULL);
        duration += 2;

        char msg_s[BUFSIZ];

        int f = open(fifo_name, O_RDONLY | O_NONBLOCK);

        if ((f = open(fifo_name, O_WRONLY | O_NONBLOCK)) == -1){
            perror("Open Error\n");
            exit(-1);
        } else {

            strcpy(msg_s, "D");
            
            if ( write(f, msg_s, sizeof(msg_s)) == -1){
                perror("Write Error\n");
                exit(-1);
            }
        }
        close(f);
    }

    return duration;
}

void write_fifo(char* msg, char* fifo_name) {
    int f = open(fifo_name, O_RDONLY | O_NONBLOCK);

    if ((f = open(fifo_name, O_WRONLY | O_NONBLOCK)) == -1){
        perror("Open Error\n");
        exit(-1);
    } else {
        if ( write(f, msg, sizeof(msg)) == -1){
            perror("Write Error\n");
            exit(-1);
        }
    }
    close(f);
}


/*
**************************************************************
---------------------| Queue Functions | ---------------------
**************************************************************
*/

struct Queue* create_queue(struct Queue* q) {
    if (q == NULL) {
        q = (struct Queue*) malloc(sizeof(struct Queue));

        if (q == NULL)
            perror("Malloc Failed!!!");
            
        q->max_size = 20;
        q->head = 0;
        q->tail = 0;
    }
    return q;
}

void enqueue(struct Queue* q, int data) {
    q->arr[q->tail] = data;
    q->tail = (q->tail + 1) % q->max_size;
}

int dequeue(struct Queue* q) {
    int data = q->arr[q->head];
    q->head = (q->head + 1) % q->max_size;

    return data;
}

void display_queue(struct Queue* q) {
    if (is_empty_queue(q)) {
        printf("Empty Queue\n");
        fflush(NULL);
    }
    for (int i = 0; i < get_queue_size(q); i++) {
        printf("%d ", q->arr[i]);
        fflush(NULL);
    }
    printf("\n");
    fflush(NULL);
}

int get_queue_size(struct Queue* q) {
    return abs(q->tail - q->head);
}

bool is_empty_queue(struct Queue* q) {
    return abs(q->tail - q->head) == 0;
}

void clear_queue(struct Queue* q) {
    q->head = 0;
    q->tail = 0;
}

void delete_queue(struct Queue* q) {
    free(q);
}