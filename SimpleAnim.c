#include <GL/glut.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

// Global variables for ball position and movement
float ballX = -0.5f; // Initial x-coordinate of the ball (with the first team lead)
float ballY = 0.4f;  // Initial y-coordinate of the ball (with the first team lead)
float ballSpeed = 0.005f; // Speed of the ball

// Sequence of positions for the ball to follow
float ballPositions[][2] = {
    {-0.5f, 0.4f},  // Starting position with team lead
    {-0.76f, -0.1f}, // Horizontal movement to first player
    {-0.63f, -0.1f}, // Horizontal movement to second player
    {-0.5f, -0.1f}, // Horizontal movement to third player
    {-0.37f, -0.1f},  // Horizontal movement to fourth player
    {-0.24f, -0.1f},  // Horizontal movement to fifth player
    {-0.5f, 0.4f} // Back to team lead
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

// Function to update the position of the ball
void updateBallPosition() {
    // Check if the ball reaches the end of the sequence
    if (currentBallPositionIndex >= sizeof(ballPositions) / sizeof(ballPositions[0]))
        return;

    // Move the ball towards the next position
    float targetX = ballPositions[currentBallPositionIndex][0];
    float targetY = ballPositions[currentBallPositionIndex][1];

    if (ballX < targetX)
        ballX += ballSpeed;
    else if (ballX > targetX)
        ballX -= ballSpeed;

    if (ballY < targetY)
        ballY += ballSpeed;
    else if (ballY > targetY)
        ballY -= ballSpeed;

    // Check if the ball reaches the target position
    if (fabs(ballX - targetX) <= ballSpeed && fabs(ballY - targetY) <= ballSpeed) {
        currentBallPositionIndex++; // Move to the next position in the sequence

        // Check if the ball returns to the team lead
        if (currentBallPositionIndex >= sizeof(ballPositions) / sizeof(ballPositions[0])) {
            currentBallPositionIndex = 0; // Reset to start the sequence again
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
    }

    // Draw the children (players) for team 2
    for(int i = 0; i < 5; i++) {
        drawPlayer(0.15f + i * 0.18f, -0.1, 0.0f, 0.0f, 1.0f); // Team 2 players (blue)
		glColor3f(1.0f, 1.0f, 0.0f); // Set color to yellow
		char s[10];
		sprintf(s, "P%d", i+1);
		drawText(0.13f + i * 0.18f, 0.06, s);
    }

    // Draw the ball
    glColor3f(0.0f, 1.0f, 0.0f); // Set color to green
    drawCircle(ballX, ballY, 0.03f, 20); // Draw the ball at its position

    glutSwapBuffers(); // Swap the front and back frame buffers (double buffering)
}

// Main function
int main(int argc, char** argv) {
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
