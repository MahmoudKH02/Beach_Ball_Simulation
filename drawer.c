#include "headers.h"

void send_ball_a(int sig);
void send_ball_b(int sig);
void end_round(int sig);

void generate_random_color(int);

char fifos[MAX_PLAYERS][100] = {
    "/tmp/fifoA0",
    "/tmp/fifoA1",
    "/tmp/fifoA2",
    "/tmp/fifoA3",
    "/tmp/fifoA4",
    "/tmp/fifoA5",
    "/tmp/fifoB0",
    "/tmp/fifoB1",
    "/tmp/fifoB2",
    "/tmp/fifoB3",
    "/tmp/fifoB4",
    "/tmp/fifoB5"
};

int fd_pipe[2];

struct Ball balls[MAX_BALLS];

int num_balls = 0;
int wins_a = 0;
int wins_b = 0;

char msg_r[BUFSIZ];
char last_winner[BUFSIZ];

float energy_bars[MAX_PLAYERS];
int players_balls[MAX_PLAYERS]; // balls with each player
float rgb[3];

// Global variables for ball position and movement
float ballX = -0.5f; // Initial x-coordinate of the ball (with the first team lead)
float ballY = 0.4f;  // Initial y-coordinate of the ball (with the first team lead)
float ballSpeed = 0.5f; // Speed of the ball

float ballPositions[][2] = {
    {-0.5f, 0.4f},    // Starting position with team lead A
    {-0.85f, -0.1f},  // Horizontal movement to first player of team A
    {-0.67f, -0.1f},  // Horizontal movement to second player of team A
    {-0.49f, -0.1f},  // Horizontal movement to third player of team A
    {-0.31f, -0.1f},  // Horizontal movement to fourth player of team A
    {-0.13f, -0.1f},  // Horizontal movement to fifth player of team A
    {0.5f, 0.4f},     // position of team lead B
    {0.15f, -0.1f},   // Horizontal movement to first player of team B
    {0.33f, -0.1f},   // Horizontal movement to second player of team B
    {0.51f, -0.1f},   // Horizontal movement to third player of team B
    {0.69f, -0.1f},   // Horizontal movement to fourth player of team B
    {0.87f, -0.1f},   // Horizontal movement to fifth player of team B
    {-0.0f, 0.75f},   // position of parent
};

int currentBallPositionIndex = 0; // Index of the current position in the sequence

// Function to initialize the OpenGL environment
void initialize() {
    glClearColor(0.0, 0.0, 0.0, 1.0); // Set background color to black
    glEnable(GL_DEPTH_TEST); // Enable depth test for 3D rendering
    glDepthFunc(GL_LEQUAL); // Specify the depth test function
}

// Function to draw a circle
void drawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_POLYGON);
    for(int ii = 0; ii < num_segments; ii++) {
        float theta = 2.0f * 3.1415926f * ((float)ii / (float)num_segments); // Ensure proper casting
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

// Function to draw a rectangle
void drawRectangle(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

// Function to draw a rectangle filled
void drawRectangleLine(float x, float y, float width, float height) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

// Function to draw a stick figure for a player or team lead
void drawPlayer(float x, float y, float colorR, float colorG, float colorB) {
    // Set player color
    glColor3f(colorR, colorG, colorB);

    // Draw head
    drawCircle(x, y + 0.08f, 0.03f, 20);

    // Draw body
    glBegin(GL_LINES);
    glVertex2f(x, y); // Body's upper point
    glVertex2f(x, y - 0.1f); // Body's lower point
    glEnd();

    // Draw arms
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.05f); // Arms' upper point
    glVertex2f(x - 0.05f, y - 0.1f); // Left arm's lower point
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.05f); // Arms' upper point
    glVertex2f(x + 0.05f, y - 0.1f); // Right arm's lower point
    glEnd();

    // Draw legs
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x - 0.03f, y - 0.15f); // Left leg's lower point
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Legs' upper point
    glVertex2f(x + 0.03f, y - 0.15f); // Right leg's lower point
    glEnd();
}

// Function to draw a stick figure for the team lead
void drawTeamLead(float x, float y, float colorR, float colorG, float colorB) {
    // Set team lead color
    glColor3f(colorR, colorG, colorB);

    // Draw head (larger circle)
    drawCircle(x, y + 0.1f, 0.07f, 20);

    // Draw body
    drawRectangle(x - 0.02f, y - 0.04f, 0.04f, 0.2f);

    // Draw arms
    drawRectangle(x - 0.08f, y, 0.16f, 0.03f);

    // Draw legs
    drawRectangle(x - 0.02f, y - 0.16f, 0.04f, 0.2f);
}

// Function to draw the parent as a stick figure with a head, arms, and legs
void drawParent(float x, float y) {
    // Draw head
    drawCircle(x, y + 0.07f, 0.04f, 20);

    // Draw body
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.03f); // Body's upper point
    glVertex2f(x, y - 0.2f); // Body's lower point
    glEnd();

    // Draw arms
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Arms' upper point
    glVertex2f(x - 0.05f, y - 0.15f); // Left arm's lower point
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.1f); // Arms' upper point
    glVertex2f(x + 0.05f, y - 0.15f); // Right arm's lower point
    glEnd();

    // Draw legs
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.2f); // Legs' upper point
    glVertex2f(x - 0.03f, y - 0.25f); // Left leg's lower point
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(x, y - 0.2f); // Legs' upper point
    glVertex2f(x + 0.03f, y - 0.25f); // Right leg's lower point
    glEnd();
}

int ball_index(int player) {
    for (int i = 0; i < num_balls; i++) {
        if (balls[i].player_id == player)
            return i;
    }
    return -1;
}

void read_fifo() {
    // read from fifo
    int f;
    int bytes;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if ((f = open(fifos[i], O_RDONLY | O_NONBLOCK)) == -1) {
            perror("Open Error Drawer\n");
            fflush(NULL);
            exit(-1);
        } else {
            if ( (bytes = read(f, msg_r, sizeof(msg_r))) == -1) {
                perror("Read Error\n");
                fflush(NULL);
                exit(-1);
            } else if (bytes > 0) {   
                // update struct
                printf("str (%d): %s | bytes=%d\n", i, msg_r, bytes);
                fflush(NULL);
                
                strtok(msg_r, ",");

                if (strcmp(msg_r, "P") == 0) { 
                    char* string = strtok('\0', ",");
                    int target = atoi(string); // target position
                    printf("target (%d): %d\n", i, target);

                    char* strEnergy = strtok('\0', "\n");
                    energy_bars[i] = atof(strEnergy);

                    int ball_i = ball_index(i);

                    if (ball_i == -1)
                        continue;

                    balls[ball_i].player_id = target;
                    balls[ball_i].target_ball_position = target;
                    players_balls[target]++;
                    players_balls[i]--;
                } else if (strcmp(msg_r, "D") == 0) {
                    // change color...

                } else if (strcmp(msg_r, "E") == 0) {
                    char* str = strtok('\0', ","); // energy lvl
                    energy_bars[i] = atof(str);
                }
            }
        }
        close(f);
    }
}

void updateBallPosition() {
// Calculate the direction and distance to move

    read_fifo();

    for (int i = 0; i < num_balls; i++) {
        float dx = ballPositions[ balls[i].target_ball_position ][0] - balls[i].current_ball_position[0];
        float dy = ballPositions[ balls[i].target_ball_position ][1] - balls[i].current_ball_position[1];
        float distance = sqrt(dx * dx + dy * dy);

        // Move the ball towards the target position
        if (distance > ballSpeed) {
            balls[i].current_ball_position[0] += (dx / distance) * ballSpeed;
            balls[i].current_ball_position[1] += (dy / distance) * ballSpeed;
        } else {
            // If the distance is less than the speed, snap the ball to the target position
            balls[i].current_ball_position[0] = ballPositions[ balls[i].target_ball_position ][0];
            balls[i].current_ball_position[1] = ballPositions[ balls[i].target_ball_position ][1];
        }
    }

    glutPostRedisplay(); // Request redisplay to continuously update the scene
}

// Function to draw text on the screen
void drawText(float x, float y, const char *string) {
    glRasterPos2f(x, y);

    // Loop through each character of the string
    while (*string) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *string);
        string++;
    }
}

// Function to display the scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color and depth buffers
    glLoadIdentity(); // Reset the current modelview matrix

    // Draw the parent
    glColor3f(1.0, 1.0, 1.0); // Set color to white
    drawParent(-0.0f, 0.75f);
	glColor3f(1.0f, 1.0f, 0.0f); // Set color to yellow
	drawText(-0.12f, 0.9f, "The Parent");

    char s[BUFSIZ + 200];

    // Draw the team leads
    // For team 1 (red)
    drawTeamLead(-0.5f, 0.4f, 1.0f, 0.0f, 0.0f); // Team lead 1
    glColor3f(1.0f, 1.0f, 0.0f); // Set color to yellow
    drawText(-0.63f, 0.60f, "Team Lead A");
    glColor3f(1.0f, 0.647f, 0.0f); // set color to orange
    drawRectangleLine(-0.59f, 0.19f, 0.17f, 0.02f);

    // energy bars
    if (energy_bars[LEAD_A] >= 0.8)
        glColor3f(0.0f, 0.7f, 0.2f);
    else if (energy_bars[LEAD_A] < 0.4)
        glColor3f(1.0f, 0.05f, 0.0f);
    else
        glColor3f(1.0f, 0.647f, 0.0f);

    drawRectangle(-0.59f, 0.19f, 0.17f * energy_bars[0], 0.02f);
    // draw the energy level text
    sprintf(s, "E=%d", (int) (energy_bars[LEAD_A] * 100));
    glColor3f(1.0f, 1.0f, 1.0f); // Set color to yellow
    drawText(-0.55f, 0.14, s);

    // For team 2 (blue)
    drawTeamLead(0.5f, 0.4f, 0.0f, 0.0f, 1.0f); // Team lead 2
    glColor3f(1.0f, 1.0f, 0.0f); // Set color to yellow
    drawText(0.35f, 0.60f, "Team Lead B");
    glColor3f(1.0f, 0.647f, 0.0f); // set color to orange
    drawRectangleLine(0.41f, 0.19f, 0.17f, 0.02f);

    // energy bars
    if (energy_bars[LEAD_B] >= 0.8)
        glColor3f(0.0f, 0.7f, 0.2f);
    else if (energy_bars[LEAD_B] < 0.4)
        glColor3f(1.0f, 0.05f, 0.0f);
    else
        glColor3f(1.0f, 0.647f, 0.0f);
    drawRectangle(0.41f, 0.19f, 0.17f * energy_bars[6], 0.02f);

    // draw the energy level text
    sprintf(s, "E=%d", (int) (energy_bars[LEAD_B] * 100));
    glColor3f(1.0f, 1.0f, 1.0f); // Set color to yellow
    drawText(0.46f, 0.14, s);

    // Draw the children (players) for team 1
    for(int i = 0; i < 5; i++) {
        drawPlayer(-0.85f + i * 0.18f, -0.1, 1.0f, 0.0f, 0.0f); // Team 1 players (red)
		ballPositions[i+1][0] = -0.85f + i * 0.18f;
		ballPositions[i+1][1] = -0.1;
		glColor3f(1.0f, 1.0f, 0.0f); // Set color to yellow
		sprintf(s, "P%d", i+1);
		drawText(-0.88f + i * 0.18f, 0.06, s);
		glColor3f(1.0f, 0.647f, 0.0f); // set color to orange
		drawRectangleLine(-0.93f + i * 0.18f, -0.3, 0.17, 0.02);

        // energy bars
        if (energy_bars[i] >= 0.8)
            glColor3f(0.0f, 0.7f, 0.2f);
        else if (energy_bars[i] <= 0.4)
            glColor3f(1.0f, 0.05f, 0.0f);
        else
            glColor3f(1.0f, 0.647f, 0.0f);

        drawRectangle(-0.93f + i * 0.18f, -0.3, 0.17f * energy_bars[i], 0.02f);

        sprintf(s, "E=%d", (int) (energy_bars[i] * 100));
        glColor3f(1.0f, 1.0f, 1.0f); // Set color to yellow
		drawText(-0.9f + i * 0.18f, -0.35, s);
    }

    // Draw the children (players) for team 2
    for(int i = 0; i < 5; i++) {
        drawPlayer(0.15f + i * 0.18f, -0.1, 0.0f, 0.0f, 1.0f); // Team 2 players (blue)
		glColor3f(1.0f, 1.0f, 0.0f); // Set color to yellow

		sprintf(s, "P%d", i+1);
		drawText(0.13f + i * 0.18f, 0.06, s);
		glColor3f(1.0f, 0.647f, 0.0f); // set color to orange
		drawRectangleLine(0.06f + i * 0.18f, -0.3, 0.17, 0.02);
        
        // energy bars
        if (energy_bars[i+6] >= 0.8)
            glColor3f(0.0f, 0.7f, 0.2f);
        else if (energy_bars[i+6] <= 0.4)
            glColor3f(1.0f, 0.05f, 0.0f);
        else
            glColor3f(1.0f, 0.647f, 0.0f);

        drawRectangle(0.06f + i * 0.18f, -0.3, 0.17f * energy_bars[i+6], 0.02f);

        sprintf(s, "E=%d", (int) (energy_bars[i+6] * 100));
        glColor3f(1.0f, 1.0f, 1.0f); // Set color to yellow
		drawText(0.09f + i * 0.18f, -0.35, s);
    }

    glColor3f(0, 0.647f, 0.34f); // set color to orange
    sprintf(s, "Last Round Winner: %s", last_winner);
    drawText(-0.3f, -0.6f, s);

    // Draw the ball
    for (int i = 0; i < num_balls; i++) {
        glColor3f(balls[i].colorR, balls[i].colorG, balls[i].colorB); // Set color to green
        float displacement = (players_balls[ balls[i].player_id ]) * (0.01f * (i%4));
        drawCircle(balls[i].current_ball_position[0], balls[i].current_ball_position[1] + displacement, 0.03f, 20); // Draw the ball at its position
    }

    // draw the score
    glColor3f(1.0, 1.0, 1.0f); // set color to orange
    drawRectangleLine(0.32, 0.67, 0.35, 0.10);
    drawRectangleLine(-0.68, 0.67, 0.35, 0.10);

    glColor3f(0, 0.647f, 0.34f);
    sprintf(s, "WINS: %d", wins_a);
    drawText(-0.60, 0.70, s);

    glColor3f(0, 0.647f, 0.34f);
    sprintf(s, "WINS: %d", wins_b);
    drawText(0.40, 0.70, s);

    glutSwapBuffers(); // Swap the front and back frame buffers (double buffering)
}

void generate_random_color(int ball_id) {
    srand(ball_id + time(NULL));

    int red = (rand() % 150);
    int green = (rand() % 201) + 50;
    int blue = (rand() % 56) + 50;

    rgb[0] = (red / 255.0);
    rgb[1] = (green / 255.0);
    rgb[2] = (blue / 255.0); 
}


// Main function
int main(int argc, char** argv) {

    if (argc < 3) {
        perror("Not enough args");
        exit(SIGQUIT);
    }

    fd_pipe[0] = atoi(argv[1]);
    fd_pipe[1] = atoi(argv[2]);

    strcpy(last_winner, "No One Yet");
    
    // balls with team leads
    // balls with Lead A
    balls[num_balls].player_id = 0;
    balls[num_balls].target_ball_position = LEAD_A;
    balls[num_balls].current_ball_position[0] = ballPositions[12][0];
    balls[num_balls].current_ball_position[1] = ballPositions[12][1];
    
    generate_random_color(num_balls);

    balls[num_balls].colorR = rgb[0];
    balls[num_balls].colorG = rgb[1];
    balls[num_balls].colorB = rgb[2];
    num_balls++;
    players_balls[LEAD_A] = 1;
    
    // balls with Lead B
    balls[num_balls].player_id = 6;
    balls[num_balls].target_ball_position = LEAD_B;
    balls[num_balls].current_ball_position[0] = ballPositions[12][0];
    balls[num_balls].current_ball_position[1] = ballPositions[12][1];

    generate_random_color(num_balls);

    balls[num_balls].colorR = rgb[0];
    balls[num_balls].colorG = rgb[1];
    balls[num_balls].colorB = rgb[2];

    num_balls++;
    players_balls[LEAD_B] = 1;

    sleep(1);
    read_fifo();

    // set signals
    if ( signal(SIGUSR1, send_ball_a) == SIG_ERR ) {
        perror("Signale error Drawer");
        exit(SIGQUIT);
    }

    if ( signal(SIGUSR2, send_ball_b) == SIG_ERR ) {
        perror("Signale error Drawer");
        exit(SIGQUIT);
    }

    if ( signal(SIGRTMIN, end_round) == SIG_ERR ) {
        perror("Signale error Drawer");
        exit(SIGQUIT);
    }


    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Enable double buffering, RGB colors, and depth buffer
    glutInitWindowSize(950, 950); // Set the window size
    glutCreateWindow("Beach Ball Game Simulation"); // Create the window with a title

    initialize(); // Call initialization routine

    glutDisplayFunc(display); // Register the display function
    glutIdleFunc(updateBallPosition); // Register the update function

    glutMainLoop(); // Enter the GLUT event processing loop
    return 0;
}

// recieve signal
void send_ball_a(int sig) {
    balls[num_balls].player_id = LEAD_A;
    balls[num_balls].target_ball_position = LEAD_A;
    balls[num_balls].current_ball_position[0] = ballPositions[12][0];
    balls[num_balls].current_ball_position[1] = ballPositions[12][1];

    generate_random_color(num_balls);

    balls[num_balls].colorR = rgb[0];
    balls[num_balls].colorG = rgb[1];
    balls[num_balls].colorB = rgb[2];
    num_balls++;
}

// recieve signal
void send_ball_b(int sig) {
    balls[num_balls].player_id = LEAD_B;
    balls[num_balls].target_ball_position = LEAD_B;
    balls[num_balls].current_ball_position[0] = ballPositions[12][0];
    balls[num_balls].current_ball_position[1] = ballPositions[12][1];

    generate_random_color(num_balls);

    balls[num_balls].colorR = rgb[0];
    balls[num_balls].colorG = rgb[1];
    balls[num_balls].colorB = rgb[2];
    num_balls++;
}

void end_round(int sig) {
    for (int i = 0; i < num_balls; i++) {
        // reset balls
        balls[i].player_id = 0;
        balls[i].target_ball_position = 0;
        balls[i].current_ball_position[0] = ballPositions[12][0];
        balls[i].current_ball_position[1] = ballPositions[12][1];
    }
    
    balls[1].player_id = 6;
    balls[1].target_ball_position = LEAD_B;

    num_balls = 2;

    for (int i = 0; i < MAX_PLAYERS; i++)
        players_balls[i] = 0;

    last_winner[BUFSIZ];
    memset(last_winner, 0x0, sizeof(last_winner));

    read(fd_pipe[0], last_winner, sizeof(last_winner));

    if (strcmp(last_winner, "Team A") == 0)
        wins_a++;
    else if (strcmp(last_winner, "Team B") == 0)
        wins_b++;
}