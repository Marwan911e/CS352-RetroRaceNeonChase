#include <windows.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <cmath>
#include <vector>
#include <ctime>
#include <cstdio>  // Added for sprintf

// Window dimensions
const int SCR_WIDTH = 1200;
const int SCR_HEIGHT = 800;

// Camera variables
float cameraX = 0.0f;
float cameraY = 10.0f;
float cameraZ = 30.0f;
float lookX = 0.0f;
float lookY = 0.0f;
float lookZ = -1.0f;

// Animation variables
float gridOffset = 0.0f;
float vortexAngle = 0.0f;
float carPosition = -50.0f;
float lastTime = 0.0f;

// Building data structure
struct Building {
    float x, z;
    float width, height, depth;
};

// Star data structure
struct Star {
    float x, y, z;
    float brightness;
};

// Car data structure
struct Car {
    float x, z;
    bool isBlue;
};

// Collections of objects
std::vector<Building> buildings;
std::vector<Star> stars;
std::vector<Car> cars;

// Function prototypes
void init();
void display();
void reshape(int width, int height);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void drawBuilding(const Building& building, bool drawWindows);
void drawGrid(float size, int divisions);
void drawVortex(float radius, int segments, int rings);
void drawCar(const Car& car);
void drawSky();
void calculateFPS();

// FPS calculation
int frameCount = 0;
float fps = 0.0f;
float currentTime = 0.0f;
float previousTime = 0.0f;

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    glutCreateWindow("Retrowave Landscape");

    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, timer, 0); // ~60 FPS

    // Initialize OpenGL
    init();

    // Start main loop
    glutMainLoop();

    return 0;
}

void init() {
    // Set background color (deep purple)
    glClearColor(0.05f, 0.0f, 0.1f, 1.0f);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Setup light 0
    GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat position[] = { 0.0f, 50.0f, 50.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    // Enable color material mode
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Create buildings
    srand(static_cast<unsigned int>(time(NULL)));
    for (int i = 0; i < 10; i++) {
        Building b;
        b.x = -20.0f + i * 4.0f;
        b.z = -10.0f - i * 3.0f;
        b.width = 3.0f + static_cast<float>(rand()) / RAND_MAX * 5.0f;
        b.height = 10.0f + static_cast<float>(rand()) / RAND_MAX * 20.0f;
        b.depth = 3.0f + static_cast<float>(rand()) / RAND_MAX * 5.0f;
        buildings.push_back(b);

        // Mirror buildings on the right side
        Building b2;
        b2.x = 20.0f - i * 4.0f;
        b2.z = -10.0f - i * 3.0f;
        b2.width = 3.0f + static_cast<float>(rand()) / RAND_MAX * 5.0f;
        b2.height = 10.0f + static_cast<float>(rand()) / RAND_MAX * 20.0f;
        b2.depth = 3.0f + static_cast<float>(rand()) / RAND_MAX * 5.0f;
        buildings.push_back(b2);
    }

    // Create stars
    for (int i = 0; i < 200; i++) {
        Star s;
        s.x = -100.0f + static_cast<float>(rand()) / RAND_MAX * 200.0f;
        s.y = 20.0f + static_cast<float>(rand()) / RAND_MAX * 80.0f;
        s.z = -100.0f + static_cast<float>(rand()) / RAND_MAX * 50.0f;
        s.brightness = 0.5f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
        stars.push_back(s);
    }

    // Create cars
    Car car1;
    car1.x = -5.0f;
    car1.z = 0.0f;
    car1.isBlue = false; // Pink car
    cars.push_back(car1);

    Car car2;
    car2.x = 5.0f;
    car2.z = -10.0f;
    car2.isBlue = true; // Blue car
    cars.push_back(car2);

    // Initialize time
    previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void display() {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset transformations
    glLoadIdentity();

    // Set the camera position
    gluLookAt(cameraX, cameraY, cameraZ,
              cameraX + lookX, cameraY + lookY, cameraZ + lookZ,
              0.0f, 1.0f, 0.0f);

    // Draw sky with stars
    drawSky();

    // Disable lighting temporarily for neon effects
    glDisable(GL_LIGHTING);

    // Draw grid
    drawGrid(100.0f, 40);

    // Draw buildings
    for (size_t i = 0; i < buildings.size(); i++) {
        drawBuilding(buildings[i], true);
    }

    // Draw vortex
    drawVortex(20.0f, 36, 10);

    // Draw cars (apply carPosition to z coordinate when drawing)
    for (size_t i = 0; i < cars.size(); i++) {
        Car carToDraw = cars[i];
        carToDraw.z += carPosition;
        drawCar(carToDraw);
    }

    // Re-enable lighting
    glEnable(GL_LIGHTING);

    // Calculate and display FPS
    calculateFPS();

    // Swap buffers
    glutSwapBuffers();
}

void reshape(int width, int height) {
    // Set viewport to window dimensions
    glViewport(0, 0, width, height);

    // Set the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set perspective projection
    gluPerspective(45.0f, (float)width / (float)height, 0.1f, 500.0f);

    // Switch back to modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void timer(int value) {
    // Calculate delta time
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Update animations
    gridOffset += 0.5f * deltaTime;
    if (gridOffset > 1.0f) {
        gridOffset -= 1.0f;
    }

    vortexAngle += 20.0f * deltaTime;
    if (vortexAngle > 360.0f) {
        vortexAngle -= 360.0f;
    }

    carPosition += 15.0f * deltaTime;
    if (carPosition > 50.0f) {
        carPosition = -50.0f;
    }

    // Request a redisplay
    glutPostRedisplay();

    // Set the next timer callback
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    float speed = 1.0f;

    switch (key) {
        case 27: // ESC key
            exit(0);
            break;
        case 'w':
            cameraX += lookX * speed;
            cameraZ += lookZ * speed;
            break;
        case 's':
            cameraX -= lookX * speed;
            cameraZ -= lookZ * speed;
            break;
        case 'a':
            cameraX -= lookZ * speed;
            cameraZ += lookX * speed;
            break;
        case 'd':
            cameraX += lookZ * speed;
            cameraZ -= lookX * speed;
            break;
        case 'q':
            cameraY += speed;
            break;
        case 'e':
            cameraY -= speed;
            break;
    }

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    float rotationSpeed = 0.05f;

    switch (key) {
        case GLUT_KEY_UP:
            lookY += rotationSpeed;
            break;
        case GLUT_KEY_DOWN:
            lookY -= rotationSpeed;
            break;
        case GLUT_KEY_LEFT:
            // Rotate look direction left
            {
                float tempX = lookX;
                lookX = lookX * cosf(rotationSpeed) - lookZ * sinf(rotationSpeed);
                lookZ = tempX * sinf(rotationSpeed) + lookZ * cosf(rotationSpeed);
            }
            break;
        case GLUT_KEY_RIGHT:
            // Rotate look direction right
            {
                float tempX = lookX;
                lookX = lookX * cosf(-rotationSpeed) - lookZ * sinf(-rotationSpeed);
                lookZ = tempX * sinf(-rotationSpeed) + lookZ * cosf(-rotationSpeed);
            }
            break;
    }

    // Normalize look vector
    float length = sqrt(lookX * lookX + lookY * lookY + lookZ * lookZ);
    lookX /= length;
    lookY /= length;
    lookZ /= length;

    glutPostRedisplay();
}

void drawBuilding(const Building& building, bool drawWindows) {
    float x = building.x;
    float z = building.z;
    float width = building.width;
    float height = building.height;
    float depth = building.depth;
    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;

    // Draw the building (neon pink)
    glColor3f(0.9f, 0.2f, 0.8f);

    // Front face
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x + halfWidth, height, z + halfDepth);
    glVertex3f(x - halfWidth, height, z + halfDepth);
    glEnd();

    // Back face
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, height, z - halfDepth);
    glVertex3f(x - halfWidth, height, z - halfDepth);
    glEnd();

    // Left face
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x - halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x - halfWidth, height, z + halfDepth);
    glVertex3f(x - halfWidth, height, z - halfDepth);
    glEnd();

    // Right face
    glBegin(GL_QUADS);
    glVertex3f(x + halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x + halfWidth, height, z + halfDepth);
    glVertex3f(x + halfWidth, height, z - halfDepth);
    glEnd();

    // Top face
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth, height, z - halfDepth);
    glVertex3f(x + halfWidth, height, z - halfDepth);
    glVertex3f(x + halfWidth, height, z + halfDepth);
    glVertex3f(x - halfWidth, height, z + halfDepth);
    glEnd();

    // Add windows (blue)
    if (drawWindows) {
        glColor3f(0.0f, 0.7f, 1.0f);

        int numWindows = static_cast<int>(height) / 2;
        for (int i = 0; i < numWindows; i++) {
            float windowY = 2.0f + i * 3.0f;
            if (windowY >= height) continue;

            // Windows on front face
            for (int j = 0; j < 3; j++) {
                float windowX = x - halfWidth + halfWidth * 0.5f + j * (width / 4.0f);
                float windowSize = 0.8f;

                glBegin(GL_QUADS);
                glVertex3f(windowX - windowSize/2, windowY - windowSize/2, z + halfDepth + 0.01f);
                glVertex3f(windowX + windowSize/2, windowY - windowSize/2, z + halfDepth + 0.01f);
                glVertex3f(windowX + windowSize/2, windowY + windowSize/2, z + halfDepth + 0.01f);
                glVertex3f(windowX - windowSize/2, windowY + windowSize/2, z + halfDepth + 0.01f);
                glEnd();
            }

            // Windows on sides (fewer)
            if (i % 2 == 0) {
                // Right side
                float windowZ = z;
                float windowSize = 0.8f;

                glBegin(GL_QUADS);
                glVertex3f(x + halfWidth + 0.01f, windowY - windowSize/2, windowZ - windowSize/2);
                glVertex3f(x + halfWidth + 0.01f, windowY - windowSize/2, windowZ + windowSize/2);
                glVertex3f(x + halfWidth + 0.01f, windowY + windowSize/2, windowZ + windowSize/2);
                glVertex3f(x + halfWidth + 0.01f, windowY + windowSize/2, windowZ - windowSize/2);
                glEnd();

                // Left side
                glBegin(GL_QUADS);
                glVertex3f(x - halfWidth - 0.01f, windowY - windowSize/2, windowZ - windowSize/2);
                glVertex3f(x - halfWidth - 0.01f, windowY - windowSize/2, windowZ + windowSize/2);
                glVertex3f(x - halfWidth - 0.01f, windowY + windowSize/2, windowZ + windowSize/2);
                glVertex3f(x - halfWidth - 0.01f, windowY + windowSize/2, windowZ - windowSize/2);
                glEnd();
            }
        }
    }
}

void drawGrid(float size, int divisions) {
    float step = size / divisions;
    float halfSize = size / 2.0f;
    float startY = 0.0f;

    // First draw the grid surface (slightly below the actual grid lines)
    glColor3f(0.05f, 0.0f, 0.1f); // Dark purple
    glBegin(GL_QUADS);
    glVertex3f(-halfSize, startY - 0.01f, -halfSize);
    glVertex3f(halfSize, startY - 0.01f, -halfSize);
    glVertex3f(halfSize, startY - 0.01f, halfSize);
    glVertex3f(-halfSize, startY - 0.01f, halfSize);
    glEnd();

    // Apply grid offset for animation
    float offsetZ = fmodf(gridOffset * step, step);

    // Draw grid lines along Z axis (pink)
    glColor3f(0.9f, 0.2f, 0.8f);
    for (int i = 0; i <= divisions; i++) {
        float x = -halfSize + i * step;
        float thickness = 0.1f;

        if (i % 5 == 0) {
            thickness = 0.3f; // Make every 5th line thicker
        }

        glBegin(GL_QUADS);
        glVertex3f(x - thickness/2, startY, -halfSize + offsetZ);
        glVertex3f(x + thickness/2, startY, -halfSize + offsetZ);
        glVertex3f(x + thickness/2, startY, halfSize + offsetZ);
        glVertex3f(x - thickness/2, startY, halfSize + offsetZ);
        glEnd();
    }

    // Draw grid lines along X axis (blue)
    glColor3f(0.0f, 0.7f, 1.0f);
    for (int i = 0; i <= divisions; i++) {
        float z = -halfSize + i * step + offsetZ;
        if (z > halfSize) continue;

        float thickness = 0.1f;
        if (i % 5 == 0) {
            thickness = 0.3f; // Make every 5th line thicker
        }

        glBegin(GL_QUADS);
        glVertex3f(-halfSize, startY, z - thickness/2);
        glVertex3f(halfSize, startY, z - thickness/2);
        glVertex3f(halfSize, startY, z + thickness/2);
        glVertex3f(-halfSize, startY, z + thickness/2);
        glEnd();
    }
}

void drawVortex(float radius, int segments, int rings) {
    float ringStep = radius / rings;

    // Move vortex to right position
    glPushMatrix();
    glTranslatef(0.0f, -20.0f, -60.0f);
    glRotatef(vortexAngle, 0.0f, 0.0f, 1.0f);

    for (int r = 0; r < rings; r++) {
        float innerRadius = r * ringStep;
        float outerRadius = (r + 1) * ringStep;
        float heightInner = r * 0.5f;
        float heightOuter = (r + 1) * 0.5f;

        for (int i = 0; i < segments; i++) {
            float angle1 = static_cast<float>(i) / segments * 2.0f * M_PI;
            float angle2 = static_cast<float>(i + 1) / segments * 2.0f * M_PI;

            float x1Inner = innerRadius * cosf(angle1);
            float z1Inner = innerRadius * sinf(angle1);
            float x2Inner = innerRadius * cosf(angle2);
            float z2Inner = innerRadius * sinf(angle2);

            float x1Outer = outerRadius * cosf(angle1);
            float z1Outer = outerRadius * sinf(angle1);
            float x2Outer = outerRadius * cosf(angle2);
            float z2Outer = outerRadius * sinf(angle2);

            // Alternate between pink and blue for the vortex rings
            if (i % 2 == 0) {
                glColor3f(0.9f, 0.2f, 0.8f); // Pink
            } else {
                glColor3f(0.0f, 0.7f, 1.0f); // Blue
            }

            // Draw the vortex segment
            glBegin(GL_QUADS);
            glVertex3f(x1Inner, heightInner, z1Inner);
            glVertex3f(x1Outer, heightOuter, z1Outer);
            glVertex3f(x2Outer, heightOuter, z2Outer);
            glVertex3f(x2Inner, heightInner, z2Inner);
            glEnd();
        }
    }

    glPopMatrix();
}

void drawCar(const Car& car) {
    float x = car.x;
    float z = car.z;
    float carLength = 4.0f;
    float carWidth = 2.0f;
    float carHeight = 1.5f;

    // Choose car color
    if (car.isBlue) {
        glColor3f(0.0f, 0.7f, 1.0f); // Blue
    } else {
        glColor3f(0.9f, 0.2f, 0.8f); // Pink
    }

    // Car body
    glBegin(GL_QUADS);
    // Bottom
    glVertex3f(x - carWidth/2, 0.5f, z - carLength/2);
    glVertex3f(x + carWidth/2, 0.5f, z - carLength/2);
    glVertex3f(x + carWidth/2, 0.5f, z + carLength/2);
    glVertex3f(x - carWidth/2, 0.5f, z + carLength/2);

    // Top
    glVertex3f(x - carWidth/2, 0.5f + carHeight, z - carLength/2);
    glVertex3f(x + carWidth/2, 0.5f + carHeight, z - carLength/2);
    glVertex3f(x + carWidth/2, 0.5f + carHeight, z + carLength/2);
    glVertex3f(x - carWidth/2, 0.5f + carHeight, z + carLength/2);

    // Front
    glVertex3f(x - carWidth/2, 0.5f, z + carLength/2);
    glVertex3f(x + carWidth/2, 0.5f, z + carLength/2);
    glVertex3f(x + carWidth/2, 0.5f + carHeight, z + carLength/2);
    glVertex3f(x - carWidth/2, 0.5f + carHeight, z + carLength/2);

    // Back
    glVertex3f(x - carWidth/2, 0.5f, z - carLength/2);
    glVertex3f(x + carWidth/2, 0.5f, z - carLength/2);
    glVertex3f(x + carWidth/2, 0.5f + carHeight, z - carLength/2);
    glVertex3f(x - carWidth/2, 0.5f + carHeight, z - carLength/2);

    // Left side
    glVertex3f(x - carWidth/2, 0.5f, z - carLength/2);
    glVertex3f(x - carWidth/2, 0.5f, z + carLength/2);
    glVertex3f(x - carWidth/2, 0.5f + carHeight, z + carLength/2);
    glVertex3f(x - carWidth/2, 0.5f + carHeight, z - carLength/2);

    // Right side
    glVertex3f(x + carWidth/2, 0.5f, z - carLength/2);
    glVertex3f(x + carWidth/2, 0.5f, z + carLength/2);
    glVertex3f(x + carWidth/2, 0.5f + carHeight, z + carLength/2);
    glVertex3f(x + carWidth/2, 0.5f + carHeight, z - carLength/2);
    glEnd();

    // Windows (black)
    glColor3f(0.1f, 0.1f, 0.1f);

    // Front windshield
    glBegin(GL_QUADS);
    glVertex3f(x - carWidth/2 + 0.2f, 0.5f + carHeight*0.6f, z + carLength/2 - 0.2f);
    glVertex3f(x + carWidth/2 - 0.2f, 0.5f + carHeight*0.6f, z + carLength/2 - 0.2f);
    glVertex3f(x + carWidth/2 - 0.2f, 0.5f + carHeight - 0.1f, z + carLength/2 - 0.6f);
    glVertex3f(x - carWidth/2 + 0.2f, 0.5f + carHeight - 0.1f, z + carLength/2 - 0.6f);
    glEnd();

    // Rear windshield
    glBegin(GL_QUADS);
    glVertex3f(x - carWidth/2 + 0.2f, 0.5f + carHeight*0.6f, z - carLength/2 + 0.2f);
    glVertex3f(x + carWidth/2 - 0.2f, 0.5f + carHeight*0.6f, z - carLength/2 + 0.2f);
    glVertex3f(x + carWidth/2 - 0.2f, 0.5f + carHeight - 0.1f, z - carLength/2 + 0.6f);
    glVertex3f(x - carWidth/2 + 0.2f, 0.5f + carHeight - 0.1f, z - carLength/2 + 0.6f);
    glEnd();

    // Tail lights (red)
    glColor3f(1.0f, 0.1f, 0.1f);

    glBegin(GL_QUADS);
    // Left tail light
    glVertex3f(x - carWidth/2 + 0.2f, 0.5f + carHeight*0.3f, z - carLength/2 + 0.01f);
    glVertex3f(x - carWidth/2 + 0.6f, 0.5f + carHeight*0.3f, z - carLength/2 + 0.01f);
    glVertex3f(x - carWidth/2 + 0.6f, 0.5f + carHeight*0.7f, z - carLength/2 + 0.01f);
    glVertex3f(x - carWidth/2 + 0.2f, 0.5f + carHeight*0.7f, z - carLength/2 + 0.01f);

    // Right tail light
    glVertex3f(x + carWidth/2 - 0.2f, 0.5f + carHeight*0.3f, z - carLength/2 + 0.01f);
    glVertex3f(x + carWidth/2 - 0.6f, 0.5f + carHeight*0.3f, z - carLength/2 + 0.01f);
    glVertex3f(x + carWidth/2 - 0.6f, 0.5f + carHeight*0.7f, z - carLength/2 + 0.01f);
    glVertex3f(x + carWidth/2 - 0.2f, 0.5f + carHeight*0.7f, z - carLength/2 + 0.01f);
    glEnd();

    // Glowing effect under the car
    if (car.isBlue) {
        glColor3f(0.0f, 0.7f, 1.0f); // Blue glow
    } else {
        glColor3f(0.9f, 0.2f, 0.8f); // Pink glow
    }

    glBegin(GL_QUADS);
    glVertex3f(x - carWidth/2, 0.1f, z - carLength/2);
    glVertex3f(x + carWidth/2, 0.1f, z - carLength/2);
    glVertex3f(x + carWidth/2, 0.1f, z + carLength/2);
    glVertex3f(x - carWidth/2, 0.1f, z + carLength/2);
    glEnd();
}

void drawSky() {
    // Draw stars
    glPointSize(2.0);
    glBegin(GL_POINTS);

    for (size_t i = 0; i < stars.size(); i++) {
        const Star& star = stars[i];
        glColor3f(star.brightness, star.brightness, star.brightness);
        glVertex3f(star.x, star.y, star.z);
    }

    glEnd();
}

void calculateFPS() {
    frameCount++;

    currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float timeInterval = currentTime - previousTime;

    if (timeInterval >= 1.0f) {
        fps = frameCount / timeInterval;
        previousTime = currentTime;
        frameCount = 0;

        // Display FPS in window title
        char title[64];
        snprintf(title, sizeof(title), "Retrowave Landscape - FPS: %.1f", fps);
        glutSetWindowTitle(title);
    }
}
