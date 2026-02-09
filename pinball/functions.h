#include "glut.h"
#include <cmath>
#include <iostream>

using namespace std;

#define PI 3.14159265

float x[] = { -600, -600, -100 }, y[] = { -250, -150, -350 }, X[3], Y[3];
float a[] = { 600, 600, 100 }, b[] = { -250, -150, -350 }, A[3], B[3];

float ballX = 700, ballY = -300;
float ballRadius = 30;
float xMin = -750, xMax = 750, yMin = -375, yMax = 375;

int rotationFlag = 0, ballFlag = 0, rotCountUp = 0, rotCountDown = 0, bounceCount = 0;
float xSpeed = -7.0f, ySpeed = 8.0f;
float angle = 0, theta1, theta2;
bool atMax = false;
int score = 0;

const float GRAVITY = 0.25f;
const float FRICTION = 0.995f;
const float MAX_SPEED = 15.0f;
const float FLIPPER_BOUNCE = 1.5f;
const float WALL_DAMPING = 0.85f;

void drawPoint(float, float);
void midPointCircleFill(float, float, float);
void lineDDA(float, float, float, float);
void edge_detect(float, float, float, float, int*, int*);
void scanFillTriangle(float, float, float, float, float, float);
void scanFill(float, float, float, float, float, float, float, float);

void resetBall();
void launchBall();
void bounceBall();
bool checkLineCollision(float, float, float, float);
bool checkTriangleCollision(float*, float*);

void resetFlippers();
void drawFlippers();
void flipperRotation();
void drawBoundaries();
void drawBackground();
void drawScoreDisplay();

void setGradientColor(float t) {
    glColor3f(0.05f + t * 0.15f, 0.1f + t * 0.3f, 0.5f + t * 0.4f);
}

void drawPoint(float x, float y) {
    glPointSize(1.0);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

void midPointCircleFill(float r, float X, float Y) {
    float x = 0;
    float y = r;
    float decision = 5.0f / 4.0f - r;

    while (y > x) {
        if (decision < 0) {
            x++;
            decision += 2 * x + 1;
        }
        else {
            y--;
            x++;
            decision += 2 * (x - y) + 1;
        }
        for (float i = -x; i <= x; i++) {
            for (float j = -y; j <= y; j++) {
                float dist = sqrt(i * i + j * j);
                float intensity = 1.0f - (dist / r) * 0.25f;
                glColor3f(intensity, intensity, intensity);
                drawPoint(i + X, j + Y);
                drawPoint(j + X, i + Y);
            }
        }
    }
}

void lineDDA(float x0, float y0, float xEnd, float yEnd) {
    float dx = xEnd - x0;
    float dy = yEnd - y0;
    float steps, xIncrement, yIncrement, x = x0, y = y0;

    if (fabs(dx) > fabs(dy)) {
        steps = fabs(dx);
    }
    else {
        steps = fabs(dy);
    }
    xIncrement = float(dx) / float(steps);
    yIncrement = float(dy) / float(steps);
    drawPoint(round(x), round(y));

    for (int k = 0; k < steps; k++) {
        x += xIncrement;
        y += yIncrement;
        drawPoint(round(x), round(y));
    }
}

void edge_detect(float x1a, float y1a, float x2a, float y2a, int* le, int* re) {
    float temp, x, mx;

    if (y1a > y2a) {
        temp = x1a, x1a = x2a, x2a = temp;
        temp = y1a, y1a = y2a, y2a = temp;
    }

    if (y1a == y2a)
        mx = x2a - x1a;
    else
        mx = (x2a - x1a) / (y2a - y1a);

    x = x1a;

    for (int i = int(y1a) + abs(yMin); i <= (int)y2a + abs(yMin); i++) {
        if (x < (float)le[i])
            le[i] = (int)x;
        if (x > (float)re[i])
            re[i] = (int)x;
        x += mx;
    }
}

void scanFillTriangle(float x1b, float y1b, float x2b, float y2b, float x3b, float y3b) {
    int* le = new int[abs(yMin) + abs(yMax) + 1];
    int* re = new int[abs(yMin) + abs(yMax) + 1];

    for (int i = 0; i < yMax - yMin + 1; i++)
        le[i] = xMax;
    for (int j = 0; j < yMax - yMin + 1; j++)
        re[j] = xMin;

    edge_detect(x1b, y1b, x2b, y2b, le, re);
    edge_detect(x2b, y2b, x3b, y3b, le, re);
    edge_detect(x3b, y3b, x1b, y1b, le, re);

    for (int j = 0; j < yMax - yMin + 1; j++) {
        if (le[j] <= re[j])
            for (int i = le[j]; i < re[j]; i++)
                drawPoint(i, j - abs(yMin));
    }

    delete[] le;
    delete[] re;
}

void scanFill(float x1b, float y1b, float x2b, float y2b, float x3b, float y3b, float x4b, float y4b) {
    int* le = new int[abs(yMin) + abs(yMax) + 1];
    int* re = new int[abs(yMin) + abs(yMax) + 1];

    for (int i = 0; i < yMax - yMin + 1; i++)
        le[i] = xMax;
    for (int j = 0; j < yMax - yMin + 1; j++)
        re[j] = xMin;

    edge_detect(x1b, y1b, x2b, y2b, le, re);
    edge_detect(x2b, y2b, x3b, y3b, le, re);
    edge_detect(x3b, y3b, x4b, y4b, le, re);
    edge_detect(x4b, y4b, x1b, y1b, le, re);

    for (int j = 0; j < yMax - yMin + 1; j++) {
        if (le[j] <= re[j])
            for (int i = le[j]; i < re[j]; i++)
                drawPoint(i, j - abs(yMin));
    }

    delete[] le;
    delete[] re;
}

bool checkLineCollision(float x1, float y1, float x2, float y2) {
    float A = ballY - y1;
    float B = ballX - x1;
    float C = y2 - y1;
    float D = x2 - x1;

    float dot = A * C + B * D;
    float len_sq = C * C + D * D;
    float param = (len_sq != 0) ? dot / len_sq : -1;

    float xx, yy;

    if (param < 0) {
        xx = x1;
        yy = y1;
    }
    else if (param > 1) {
        xx = x2;
        yy = y2;
    }
    else {
        xx = x1 + param * D;
        yy = y1 + param * C;
    }

    float dx = ballX - xx;
    float dy = ballY - yy;
    float distance = sqrt(dx * dx + dy * dy);

    return distance < ballRadius;
}

bool checkTriangleCollision(float* tx, float* ty) {
    if (checkLineCollision(tx[0], ty[0], tx[1], ty[1])) return true;
    if (checkLineCollision(tx[1], ty[1], tx[2], ty[2])) return true;
    if (checkLineCollision(tx[2], ty[2], tx[0], ty[0])) return true;
    return false;
}

void resetBall() {
    ballFlag = 0;
    bounceCount = 0;
    ballX = 700;
    ballY = -300;
    xSpeed = -7.0f;
    ySpeed = 8.0f;
    score = 0;
}

void launchBall() {
    midPointCircleFill(ballRadius, ballX, ballY);

    if (ballFlag == 1 && bounceCount == 0) {
        if (ballY < 300) {
            ballY += ySpeed;
            ballX += xSpeed;
        }
        else {
            bounceCount = 1;
        }
    }
}

void bounceBall() {
    if (ballFlag == 0 || bounceCount == 0) return;

    ySpeed -= GRAVITY;
    xSpeed *= FRICTION;
    ySpeed *= FRICTION;

    if (fabs(xSpeed) > MAX_SPEED) xSpeed = (xSpeed > 0) ? MAX_SPEED : -MAX_SPEED;
    if (fabs(ySpeed) > MAX_SPEED) ySpeed = (ySpeed > 0) ? MAX_SPEED : -MAX_SPEED;

    ballX += xSpeed;
    ballY += ySpeed;

    if (ballY + ballRadius >= yMax) {
        ballY = yMax - ballRadius - 1;
        ySpeed = -fabs(ySpeed) * WALL_DAMPING;
        score += 5;
    }

    if (ballX + ballRadius >= xMax) {
        ballX = xMax - ballRadius - 1;
        xSpeed = -fabs(xSpeed) * WALL_DAMPING;
    }
    else if (ballX - ballRadius <= xMin) {
        ballX = xMin + ballRadius + 1;
        xSpeed = fabs(xSpeed) * WALL_DAMPING;
    }

    if (ballX - ballRadius <= -600 && ballX > xMin) {
        if (ballY > -250 && ballY < -150) {
            ballX = -600 + ballRadius + 1;
            xSpeed = fabs(xSpeed) * WALL_DAMPING;
        }
    }

    if (ballX + ballRadius >= 600 && ballX < xMax) {
        if (ballY > -250 && ballY < -150) {
            ballX = 600 - ballRadius - 1;
            xSpeed = -fabs(xSpeed) * WALL_DAMPING;
        }
    }

    if (bounceCount >= 25) {
        if (ballX - ballRadius <= -610 && ballX > xMin) {
            if (ballY >= 235 && ballY < yMax) {
                ballX = -610 + ballRadius + 1;
                xSpeed = fabs(xSpeed) * 0.7f;
                ySpeed *= 0.9f;
                score += 25;
            }
        }

        if (ballX + ballRadius >= 610 && ballX < xMax) {
            if (ballY >= 235 && ballY < yMax) {
                ballX = 610 - ballRadius - 1;
                xSpeed = -fabs(xSpeed) * 0.7f;
                ySpeed *= 0.9f;
                score += 25;
            }
        }
    }

    float* currentX = (rotationFlag == 1) ? X : x;
    float* currentA = (rotationFlag == 1) ? A : a;

    if (ballY - ballRadius < -360) {
        if (ballX > currentX[2] && ballX < currentA[2]) {
            cout << "Ball Lost! Final Score: " << score << "\n";
            resetBall();
        }
    }

    if (ballY - ballRadius < yMin) {
        cout << "Ball Lost! Final Score: " << score << "\n";
        resetBall();
    }

    ++bounceCount;
}

void resetFlippers() {
    rotationFlag = 0;
    atMax = false;
    angle = 0;
    rotCountUp = 0;
    rotCountDown = 0;
}

void drawFlippers() {
    glColor3f(1.0, 0.3, 0.4);

    lineDDA(x[0], y[0], x[1], y[1]);
    lineDDA(x[1], y[1], x[2], y[2]);
    lineDDA(x[2], y[2], x[0], y[0]);

    lineDDA(a[0], b[0], a[1], b[1]);
    lineDDA(a[1], b[1], a[2], b[2]);
    lineDDA(a[2], b[2], a[0], b[0]);

    scanFillTriangle(x[0], y[0], x[1], y[1], x[2], y[2]);
    scanFillTriangle(a[0], b[0], a[1], b[1], a[2], b[2]);
}

void flipperRotation() {
    if (rotationFlag == 1) {
        glColor3f(1.0, 0.5, 0.2);
        theta1 = angle * PI / 180.0f;
        theta2 = (-angle) * PI / 180.0f;

        for (int i = 0; i < 3; i++) {
            x[i] += 600;
            y[i] += 200;
            X[i] = (x[i] * cos(theta1)) - (y[i] * sin(theta1));
            Y[i] = (x[i] * sin(theta1)) + (y[i] * cos(theta1));
            x[i] -= 600;
            y[i] -= 200;
            X[i] -= 600;
            Y[i] -= 200;

            a[i] -= 600;
            b[i] += 200;
            A[i] = (a[i] * cos(theta2)) - (b[i] * sin(theta2));
            B[i] = (a[i] * sin(theta2)) + (b[i] * cos(theta2));
            a[i] += 600;
            b[i] -= 200;
            A[i] += 600;
            B[i] -= 200;
        }

        lineDDA(X[0], Y[0], X[1], Y[1]);
        lineDDA(X[1], Y[1], X[2], Y[2]);
        lineDDA(X[2], Y[2], X[0], Y[0]);
        scanFillTriangle(X[0], Y[0], X[1], Y[1], X[2], Y[2]);

        lineDDA(A[0], B[0], A[1], B[1]);
        lineDDA(A[1], B[1], A[2], B[2]);
        lineDDA(A[2], B[2], A[0], B[0]);
        scanFillTriangle(A[0], B[0], A[1], B[1], A[2], B[2]);

        if (bounceCount > 0) {
            bool leftCollision = checkTriangleCollision(X, Y);
            bool rightCollision = checkTriangleCollision(A, B);

            if (leftCollision || rightCollision) {
                ySpeed = fabs(ySpeed) * FLIPPER_BOUNCE + 3.0f;

                if (leftCollision) {
                    xSpeed += 3.0f;
                }
                else {
                    xSpeed -= 3.0f;
                }

                score += 10;
            }
        }

        if (!atMax) {
            if (angle < 35) {
                angle += 3.5f;
                ++rotCountUp;
                if (rotCountUp >= 10) {
                    atMax = true;
                }
            }
            else {
                atMax = true;
            }
        }

        if (atMax) {
            if (angle > 0) {
                angle -= 3.5f;
                ++rotCountDown;
                if (rotCountDown >= 10) {
                    resetFlippers();
                }
            }
            else {
                resetFlippers();
            }
        }
    }
    else {
        drawFlippers();

        if (bounceCount > 0) {
            bool leftCollision = checkTriangleCollision(x, y);
            bool rightCollision = checkTriangleCollision(a, b);

            if (leftCollision || rightCollision) {
                ySpeed = fabs(ySpeed) * 1.2f;

                if (leftCollision) {
                    xSpeed += 1.5f;
                }
                else {
                    xSpeed -= 1.5f;
                }

                score += 5;
            }
        }
    }
}

void drawBackground() {
    for (int i = yMin; i <= yMax; i += 4) {
        float t = (float)(i - yMin) / (float)(yMax - yMin);
        setGradientColor(t);
        lineDDA(xMin, i, xMax, i);
    }
}

void drawScoreDisplay() {
    if (score <= 0) return;

    int numDots = score / 10;
    if (numDots > 25) numDots = 25;

    for (int i = 0; i < numDots; i++) {
        float xPos = -700 + i * 60;
        float yPos = 345;

        float colorIntensity = 1.0f - (i * 0.02f);
        glColor3f(1.0f, colorIntensity, 0.0f);

        midPointCircleFill(7, xPos, yPos);
    }
}

void drawBoundaries() {
    if (bounceCount > 0) {
        glColor3f(0.8, 0.7, 0.5);

        lineDDA(-600, -150, xMin, -150);
        lineDDA(-600, -250, xMin, -250);
        lineDDA(xMin, -150, xMin, -250);
        lineDDA(-600, -150, -600, -250);
        scanFill(-600, -150, xMin, -150, xMin, -250, -600, -250);

        lineDDA(600, -150, xMax, -150);
        lineDDA(600, -250, xMax, -250);
        lineDDA(xMax, -150, xMax, -250);
        lineDDA(600, -150, 600, -250);
        scanFill(600, -150, xMax, -150, xMax, -250, 600, -250);
    }

    if (bounceCount >= 25) {
        glColor3f(1.0, 0.85, 0.0);

        lineDDA(xMin, 235, -610, 235);
        lineDDA(-610, 235, -610, yMax);
        lineDDA(xMin, 235, xMin, yMax);
        lineDDA(-610, yMax, xMin, yMax);
        scanFill(xMin, yMax, -610, yMax, -610, 235, xMin, 235);

        lineDDA(xMax, 235, 610, 235);
        lineDDA(610, 235, 610, yMax);
        lineDDA(xMax, 235, xMax, yMax);
        lineDDA(610, yMax, xMax, yMax);
        scanFill(xMax, yMax, 610, yMax, 610, 235, xMax, 235);
    }
}