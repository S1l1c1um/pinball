#include <windows.h>
#include "glut.h"
#include "functions.h"

unsigned int refreshTime = 32;

void initOpenGL();
void timer(int);
void reshape(GLsizei, GLsizei);
void keyPressed(unsigned char, int, int);
void renderScene();

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1000, 500);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Pinball Game");
    initOpenGL();

    glutDisplayFunc(renderScene);
    glutTimerFunc(0, timer, 0);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyPressed);
    glutMainLoop();

    return 0;
}

void initOpenGL() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    gluOrtho2D(-750.0, 750.0, -375.0, 375.0);
}

void reshape(GLsizei width, GLsizei height) {
    glViewport(0, 0, width, height);
}

void keyPressed(unsigned char c, int x, int y) {
    if (c == 'z' || c == 'Z') {
        rotationFlag = 1;
    }
    if (c == 32) {
        ballFlag = 1;
    }
    if (c == 27) {
        exit(0);
    }
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(refreshTime, timer, 0);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawBackground();
    drawBoundaries();
    flipperRotation();
    launchBall();
    bounceBall();
    drawScoreDisplay();

    glutSwapBuffers();
}