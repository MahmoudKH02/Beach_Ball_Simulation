#include "headers.h"

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

struct ball balls[30];
int num_balls = 0;

char msg_r[BUFSIZ];

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
    for (int i = 0; i < 30; i++) {
        if (balls[i].player_id == player)
            return i;
    }
    return 0;
}

// array of structs
// struct: ballNumber, player (position), bool to_who (to other team or to next player)
// Function to update the position of the ball
// fifo A5 --> P,0 --> clean ball
// fifo A5 --> D --> dropped ball
void updateBallPosition() {
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
                strtok(msg_r, ",");
                
                if (strcmp(msg_r, "P") == 0) { // pass ball
                    char* str = strtok('\0', ",");

                    int target = atoi(str); // target position // 2
                    int ball_i = 0;

                    balls[ball_i].player_id = target;
                    balls[ball_i].target_ball_position = target;

                } else if (strcmp(msg_r, "D") == 0) {
                    // change color...
                }
            }
        }
        close(f);
    }

    for (int i = 0; i < num_balls; i++) {
        printf("Ball (%d): current=%f, target=%d\n", i, balls[i].current_ball_position[0], balls[i].target_ball_position);
        fflush(NULL);
    }

// Calculate the direction and distance to move
    float dx = ballPositions[ balls[0].target_ball_position ][0] - balls[0].current_ball_position[0];
    float dy = ballPositions[ balls[0].target_ball_position ][1] - balls[0].current_ball_position[1];
    float distance = sqrt(dx * dx + dy * dy);

    // Move the ball towards the target position
    if (distance > ballSpeed) {
        balls[0].current_ball_position[0] += (dx / distance) * ballSpeed;
        balls[0].current_ball_position[1] += (dy / distance) * ballSpeed;
    } else {
        // If the distance is less than the speed, snap the ball to the target position
        balls[0].current_ball_position[0] = ballPositions[ balls[0].target_ball_position ][0];
        balls[0].current_ball_position[1] = ballPositions[ balls[0].target_ball_position ][1];
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

    // Draw the team leads
    // For team 1 (red)
    drawTeamLead(-0.5f, 0.4f, 1.0f, 0.0f, 0.0f); // Team lead 1
	glColor3f(1.0f, 1.0f, 0.0f); // Set color to yellow
	drawText(-0.63f, 0.60f, "Team Lead A");
    // For team 2 (blue)
    drawTeamLead(0.5f, 0.4f, 0.0f, 0.0f, 1.0f); // Team lead 2
	glColor3f(1.0f, 1.0f, 0.0f); // Set color to yellow
	drawText(0.35f, 0.60f, "Team Lead B");

    // Draw the children (players) for team 1
    for(int i = 0; i < 5; i++) {
        drawPlayer(-0.85f + i * 0.18f, -0.1, 1.0f, 0.0f, 0.0f); // Team 1 players (red)
		ballPositions[i+1][0] = -0.85f + i * 0.18f;
		ballPositions[i+1][1] = -0.1;
		glColor3f(1.0f, 1.0f, 0.0f); // Set color to yellow
		char s[10];
		sprintf(s, "P%d", i+1);
		drawText(-0.88f + i * 0.18f, 0.06, s);
		glColor3f(1.0f, 0.647f, 0.0f); // set color to orange
		drawRectangleLine(-0.93f + i * 0.18f, -0.3, 0.17, 0.02);
    }

    // Draw the children (players) for team 2
    for(int i = 0; i < 5; i++) {
        drawPlayer(0.15f + i * 0.18f, -0.1, 0.0f, 0.0f, 1.0f); // Team 2 players (blue)
		glColor3f(1.0f, 1.0f, 0.0f); // Set color to yellow
		char s[10];
		sprintf(s, "P%d", i+1);
		drawText(0.13f + i * 0.18f, 0.06, s);
		glColor3f(1.0f, 0.647f, 0.0f); // set color to orange
		drawRectangleLine(0.06f + i * 0.18f, -0.3, 0.17, 0.02);
    }


    // Draw the ball
    glColor3f(0.0f, 1.0f, 0.0f); // Set color to green
    drawCircle(balls[0].current_ball_position[0], balls[0].current_ball_position[1], 0.03f, 20); // Draw the ball at its position

    glutSwapBuffers(); // Swap the front and back frame buffers (double buffering)
}

// Main function
int main(int argc, char** argv) {
    sleep(5);
    
    // balls with team leads
    // balls with Lead A
    balls[num_balls].target_ball_position = 0;
    balls[num_balls].current_ball_position[0] = ballPositions[LEAD_A][0];
    balls[num_balls].current_ball_position[1] = ballPositions[LEAD_A][1];
    num_balls++;
    
    // balls with Lead B
    balls[num_balls].target_ball_position = LEAD_B;
    balls[num_balls].current_ball_position[0] = ballPositions[LEAD_B][0];
    balls[num_balls].current_ball_position[1] = ballPositions[LEAD_B][1];
    num_balls++;


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
