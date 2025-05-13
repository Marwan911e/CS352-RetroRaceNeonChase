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

// Maximum number of cars
#define MAX_CARS 10

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
float carSpeeds[MAX_CARS] = {0.0f};
float buildingPulse = 0.0f;
float sunPosition = -20.0f;
bool dayNightCycle = false; // Changed to false to remove day cycle
float skyHue = 0.0f;

// Building data structure
struct Building {
    float x, z;
    float width, height, depth;
};

// Enhanced Star data structure
struct Star {
    float x, y, z;
    float brightness;
    float size;
    int colorType; // 0=white/blue, 1=yellow/red, 2=special
};

// New Galaxy structure
struct Galaxy {
    float x, y, z;
    float rotation;
    float size;
    int type; // 0=spiral, 1=elliptical
};

// Car data structure
struct Car {
    float x, z;
    bool isBlue;
};

// Collections of objects
std::vector<Building> buildings;
std::vector<Star> stars;
std::vector<Galaxy> galaxies;
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
void drawMoon();
void drawGalaxies();
void calculateFPS();
void initCarSpeeds();
void initStars();
void initGalaxies();
void improveGraphicsQuality();

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
    glutCreateWindow("Enhanced Retrowave Landscape");

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
    glClearColor(0.02f, 0.0f, 0.05f, 1.0f);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Setup light 0
    GLfloat ambientLight[] = { 0.1f, 0.1f, 0.2f, 1.0f };
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 1.0f, 1.0f };
    GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat position[] = { -10.0f, 20.0f, 10.0f, 1.0f };

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

    // Initialize stars
    initStars();

    // Initialize galaxies
    initGalaxies();

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

    // Add some additional cars with random positions
    for (int i = 0; i < 3; i++) {
        Car newCar;
        newCar.x = -8.0f + static_cast<float>(rand()) / RAND_MAX * 16.0f;
        newCar.z = -30.0f + static_cast<float>(rand()) / RAND_MAX * 60.0f;
        newCar.isBlue = (rand() % 2 == 0);
        cars.push_back(newCar);
    }

    // Initialize car speeds
    initCarSpeeds();

    // Improve graphics quality
    improveGraphicsQuality();

    // Initialize time
    previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void initStars() {
    stars.clear();
    for (int i = 0; i < 300; i++) { // Increased number of stars
        Star s;
        s.x = -150.0f + static_cast<float>(rand()) / RAND_MAX * 300.0f;
        s.y = 20.0f + static_cast<float>(rand()) / RAND_MAX * 80.0f;
        s.z = -150.0f + static_cast<float>(rand()) / RAND_MAX * 100.0f;
        s.brightness = 0.5f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
        s.size = 1.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
        s.colorType = rand() % 10; // Different star colors
        stars.push_back(s);
    }
}

void initGalaxies() {
    for (int i = 0; i < 3; i++) {
        Galaxy g;
        g.x = -80.0f + static_cast<float>(rand()) / RAND_MAX * 160.0f;
        g.y = 30.0f + static_cast<float>(rand()) / RAND_MAX * 60.0f;
        g.z = -90.0f + static_cast<float>(rand()) / RAND_MAX * 20.0f;
        g.rotation = static_cast<float>(rand()) / RAND_MAX * 360.0f;
        g.size = 10.0f + static_cast<float>(rand()) / RAND_MAX * 15.0f;
        g.type = rand() % 2;
        galaxies.push_back(g);
    }
}

void improveGraphicsQuality() {
    // Enable anti-aliasing for smoother lines
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Improve lighting quality
    glShadeModel(GL_SMOOTH);

    // Better perspective correction
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Enhanced depth precision
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);

    // Improve light settings
    GLfloat ambientLight[] = { 0.1f, 0.1f, 0.2f, 1.0f }; // Darker blue ambient
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 1.0f, 1.0f }; // Slightly blue diffuse
    GLfloat specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat position[] = { -10.0f, 20.0f, 10.0f, 1.0f }; // Light from front-left

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    // Add a second light
    glEnable(GL_LIGHT1);
    GLfloat ambient1[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat diffuse1[] = { 0.4f, 0.2f, 0.6f, 1.0f }; // Purple hue
    GLfloat specular1[] = { 0.5f, 0.3f, 0.7f, 1.0f };
    GLfloat position1[] = { 30.0f, 20.0f, -20.0f, 1.0f }; // From back-right

    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular1);
    glLightfv(GL_LIGHT1, GL_POSITION, position1);

    // Set material properties
    GLfloat matAmbient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat matShininess[] = { 50.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
}

void initCarSpeeds() {
    for (size_t i = 0; i < MAX_CARS; i++) {
        if (i < cars.size()) {
            carSpeeds[i] = 15.0f + static_cast<float>(rand()) / RAND_MAX * 10.0f;
        } else {
            carSpeeds[i] = 0.0f;
        }
    }
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

    // Draw sky with stars and galaxies
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

    // Draw cars with their positions
    for (size_t i = 0; i < cars.size(); i++) {
        Car carToDraw = cars[i];
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

    // Update grid animation (smoother with delta time)
    gridOffset += 0.8f * deltaTime;
    if (gridOffset > 1.0f) {
        gridOffset -= 1.0f;
    }

    // Update vortex with variable speed
    float vortexSpeed = 25.0f + 15.0f * sinf(currentTime * 0.2f); // Speed varies over time
    vortexAngle += vortexSpeed * deltaTime;
    if (vortexAngle > 360.0f) {
        vortexAngle -= 360.0f;
    }

    // Update car movement with variable speeds
    for (size_t i = 0; i < cars.size(); i++) {
        // If this is the first update, initialize random speeds
        if (carSpeeds[i] == 0.0f) {
            carSpeeds[i] = 15.0f + static_cast<float>(rand()) / RAND_MAX * 10.0f;

            // Randomize car positions so they're not all in sync
            cars[i].z = -50.0f + static_cast<float>(rand()) / RAND_MAX * 100.0f;
        }

        // Move cars with their individual speeds
        cars[i].z += carSpeeds[i] * deltaTime;

        // Reset position when car goes too far
        if (cars[i].z > 50.0f) {
            cars[i].z = -50.0f;
            // Randomize speed again
            carSpeeds[i] = 15.0f + static_cast<float>(rand()) / RAND_MAX * 10.0f;

            // 20% chance to switch color when respawning
            if (rand() % 5 == 0) {
                cars[i].isBlue = !cars[i].isBlue;
            }
        }
    }

    // Update building pulse effect for windows
    buildingPulse = 0.7f + 0.3f * sinf(currentTime * 0.5f);

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
        case '+': // Add a new car
            if (cars.size() < MAX_CARS) {
                Car newCar;
                newCar.x = -5.0f + static_cast<float>(rand()) / RAND_MAX * 10.0f;
                newCar.z = -50.0f + static_cast<float>(rand()) / RAND_MAX * 100.0f;
                newCar.isBlue = (rand() % 2 == 0);
                cars.push_back(newCar);
                carSpeeds[cars.size()-1] = 0.0f; // Will be initialized in timer
            }
            break;
        case '-': // Remove a car
            if (cars.size() > 1) {
                cars.pop_back();
            }
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

    // Get current time for animations
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Building hover effect - slight vertical movement
    float buildingOffset = sinf(time * 0.5f + x * 0.1f) * 0.2f;

    // ENHANCEMENT: More detailed building with gradients
    glBegin(GL_QUADS);

    // Front face with enhanced gradient
    glColor3f(0.9f, 0.1f, 0.8f); // Brighter pink at bottom
    glVertex3f(x - halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z + halfDepth);
    glColor3f(0.5f, 0.0f, 0.9f); // Deep purple at top
    glVertex3f(x + halfWidth, height + buildingOffset, z + halfDepth);
    glVertex3f(x - halfWidth, height + buildingOffset, z + halfDepth);

    // Back face with gradient
    glColor3f(0.9f, 0.1f, 0.8f); // Brighter pink at bottom
    glVertex3f(x - halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z - halfDepth);
    glColor3f(0.5f, 0.0f, 0.9f); // Deep purple at top
    glVertex3f(x + halfWidth, height + buildingOffset, z - halfDepth);
    glVertex3f(x - halfWidth, height + buildingOffset, z - halfDepth);

    // Left face with gradient
    glColor3f(0.9f, 0.1f, 0.8f); // Brighter pink at bottom
    glVertex3f(x - halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x - halfWidth, 0.0f, z + halfDepth);
    glColor3f(0.5f, 0.0f, 0.9f); // Deep purple at top
    glVertex3f(x - halfWidth, height + buildingOffset, z + halfDepth);
    glVertex3f(x - halfWidth, height + buildingOffset, z - halfDepth);

    // Right face with gradient
    glColor3f(0.9f, 0.1f, 0.8f); // Brighter pink at bottom
    glVertex3f(x + halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z + halfDepth);
    glColor3f(0.5f, 0.0f, 0.9f); // Deep purple at top
    glVertex3f(x + halfWidth, height + buildingOffset, z + halfDepth);
    glVertex3f(x + halfWidth, height + buildingOffset, z - halfDepth);

    // Top face
    glColor3f(0.5f, 0.0f, 0.9f); // Deep purple
    glVertex3f(x - halfWidth, height + buildingOffset, z - halfDepth);
    glVertex3f(x + halfWidth, height + buildingOffset, z - halfDepth);
    glVertex3f(x + halfWidth, height + buildingOffset, z + halfDepth);
    glVertex3f(x - halfWidth, height + buildingOffset, z + halfDepth);
    glEnd();

    // ENHANCEMENT: Add building trim/edges with neon glow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Edge glow color
    float edgePulse = 0.7f + 0.3f * sinf(time * 2.0f);
    glColor4f(0.0f, 1.0f, 1.0f, 0.8f * edgePulse); // Bright cyan

    float edgeWidth = 0.15f;

    // Vertical edges
    // Front left edge
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth - edgeWidth/2, 0.0f, z + halfDepth + edgeWidth/2);
    glVertex3f(x - halfWidth + edgeWidth/2, 0.0f, z + halfDepth + edgeWidth/2);
    glVertex3f(x - halfWidth + edgeWidth/2, height + buildingOffset, z + halfDepth + edgeWidth/2);
    glVertex3f(x - halfWidth - edgeWidth/2, height + buildingOffset, z + halfDepth + edgeWidth/2);
    glEnd();

    // Front right edge
    glBegin(GL_QUADS);
    glVertex3f(x + halfWidth - edgeWidth/2, 0.0f, z + halfDepth + edgeWidth/2);
    glVertex3f(x + halfWidth + edgeWidth/2, 0.0f, z + halfDepth + edgeWidth/2);
    glVertex3f(x + halfWidth + edgeWidth/2, height + buildingOffset, z + halfDepth + edgeWidth/2);
    glVertex3f(x + halfWidth - edgeWidth/2, height + buildingOffset, z + halfDepth + edgeWidth/2);
    glEnd();

    // Top edge glow
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth - edgeWidth/2, height + buildingOffset - edgeWidth/2, z - halfDepth - edgeWidth/2);
    glVertex3f(x + halfWidth + edgeWidth/2, height + buildingOffset - edgeWidth/2, z - halfDepth - edgeWidth/2);
    glVertex3f(x + halfWidth + edgeWidth/2, height + buildingOffset + edgeWidth/2, z - halfDepth - edgeWidth/2);
    glVertex3f(x - halfWidth - edgeWidth/2, height + buildingOffset + edgeWidth/2, z - halfDepth - edgeWidth/2);
    glEnd();

    // ENHANCEMENT: Add more detailed rooftop structures
    // Add a rooftop detail - more complex antenna array
    glColor4f(0.6f, 0.0f, 0.5f, 1.0f); // Dark pink

    // Main antenna
    glBegin(GL_QUADS);
    glVertex3f(x - 0.2f, height + buildingOffset, z - 0.2f);
    glVertex3f(x + 0.2f, height + buildingOffset, z - 0.2f);
    glVertex3f(x + 0.2f, height + 5.0f + buildingOffset, z - 0.2f);
    glVertex3f(x - 0.2f, height + 5.0f + buildingOffset, z - 0.2f);

    glVertex3f(x - 0.2f, height + buildingOffset, z + 0.2f);
    glVertex3f(x + 0.2f, height + buildingOffset, z + 0.2f);
    glVertex3f(x + 0.2f, height + 5.0f + buildingOffset, z + 0.2f);
    glVertex3f(x - 0.2f, height + 5.0f + buildingOffset, z + 0.2f);

    glVertex3f(x - 0.2f, height + buildingOffset, z - 0.2f);
    glVertex3f(x - 0.2f, height + buildingOffset, z + 0.2f);
    glVertex3f(x - 0.2f, height + 5.0f + buildingOffset, z + 0.2f);
    glVertex3f(x - 0.2f, height + 5.0f + buildingOffset, z - 0.2f);

    glVertex3f(x + 0.2f, height + buildingOffset, z - 0.2f);
    glVertex3f(x + 0.2f, height + buildingOffset, z + 0.2f);
    glVertex3f(x + 0.2f, height + 5.0f + buildingOffset, z + 0.2f);
    glVertex3f(x + 0.2f, height + 5.0f + buildingOffset, z - 0.2f);
    glEnd();

    // Antenna cross bars
    glBegin(GL_QUADS);
    glVertex3f(x - 1.0f, height + 4.0f + buildingOffset, z - 0.1f);
    glVertex3f(x + 1.0f, height + 4.0f + buildingOffset, z - 0.1f);
    glVertex3f(x + 1.0f, height + 4.2f + buildingOffset, z - 0.1f);
    glVertex3f(x - 1.0f, height + 4.2f + buildingOffset, z - 0.1f);

    glVertex3f(x - 1.0f, height + 3.0f + buildingOffset, z - 0.1f);
    glVertex3f(x + 1.0f, height + 3.0f + buildingOffset, z - 0.1f);
    glVertex3f(x + 1.0f, height + 3.2f + buildingOffset, z - 0.1f);
    glVertex3f(x - 1.0f, height + 3.2f + buildingOffset, z - 0.1f);
    glEnd();

    // Rooftop blinking light
    float blinkIntensity = (sin(time * 5.0f) > 0) ? 1.0f : 0.0f; // Blink on/off
    glColor4f(1.0f, 0.2f, 0.2f, 0.8f * blinkIntensity); // Red blink
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glVertex3f(x, height + 5.1f + buildingOffset, z);
    glEnd();

    // ENHANCEMENT: Improved windows with more variety and better glow
   // Enhanced window rendering code - replace the window drawing section in drawBuilding function

// ENHANCEMENT: Improved windows with sharper edges and more clarity
// ENHANCEMENT: Improved windows with sharper edges and more clarity
if (drawWindows) {
    // Enable blending for window glow but with improved blend function
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // For sharper base windows

    // Base intensity with pulsating effect
    float windowPulse = 0.7f + 0.3f * sinf(time * 1.5f);

    // Random seed based on building position for consistent window patterns
    srand(static_cast<unsigned int>((x + z) * 1000.0f));

    int numWindowsHorizontal = static_cast<int>(width * 2) + 2;
    int numWindowsVertical = static_cast<int>(height) / 2;

    // Front face windows with improved visuals
    for (int i = 0; i < numWindowsVertical; i++) {
        for (int j = 0; j < numWindowsHorizontal; j++) {
            // Some randomness to window presence
            if (rand() % 4 == 0) continue; // 25% chance of no window

            float windowY = 2.0f + i * 2.0f;
            if (windowY >= height) continue;

            float windowX = x - halfWidth + j * (width / (numWindowsHorizontal - 1));
            if (windowX > x + halfWidth) continue;

            // Create window frame first (black border for definition)
            float windowSize = 0.5f + static_cast<float>(rand() % 4) * 0.1f; // Larger base size
            float frameBorder = 0.05f; // Border thickness

            // Draw black window frame
            glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
            glBegin(GL_QUADS);
            glVertex3f(windowX - windowSize/2, windowY - windowSize/2, z + halfDepth + 0.005f);
            glVertex3f(windowX + windowSize/2, windowY - windowSize/2, z + halfDepth + 0.005f);
            glVertex3f(windowX + windowSize/2, windowY + windowSize/2, z + halfDepth + 0.005f);
            glVertex3f(windowX - windowSize/2, windowY + windowSize/2, z + halfDepth + 0.005f);
            glEnd();

            // Various window types with different colors and behaviors
            int windowType = rand() % 10;

            // Draw the inner illuminated part (slightly smaller than frame)
            if (windowType < 5) {
                // Standard blue windows with enhanced glow
                float blueIntensity = windowPulse * (0.8f + 0.2f * sinf(time * 3.0f + i * j * 0.1f));
                glColor4f(0.0f, 0.7f * blueIntensity, 1.0f * blueIntensity, 0.95f); // More opacity
            }
            else if (windowType < 8) {
                // Yellow/white windows (apartment lights)
                float yellowIntensity = 0.5f + 0.5f * sinf(time * 2.0f + i * 0.1f + j * 0.2f);
                glColor4f(1.0f * yellowIntensity, 0.9f * yellowIntensity, 0.5f * yellowIntensity, 0.95f);
            }
            else if (windowType == 8) {
                // Flickering red windows (alarm/warning)
                float redFlicker = (sin(time * 15.0f) > 0) ? 1.0f : 0.2f;
                glColor4f(1.0f * redFlicker, 0.1f * redFlicker, 0.1f * redFlicker, 0.95f);
            }
            else {
                // Purple/magenta windows
                glColor4f(1.0f * windowPulse, 0.0f, 0.8f * windowPulse, 0.95f);
            }

            // Draw window inner (slightly smaller than frame)
            glBegin(GL_QUADS);
            glVertex3f(windowX - windowSize/2 + frameBorder, windowY - windowSize/2 + frameBorder, z + halfDepth + 0.01f);
            glVertex3f(windowX + windowSize/2 - frameBorder, windowY - windowSize/2 + frameBorder, z + halfDepth + 0.01f);
            glVertex3f(windowX + windowSize/2 - frameBorder, windowY + windowSize/2 - frameBorder, z + halfDepth + 0.01f);
            glVertex3f(windowX - windowSize/2 + frameBorder, windowY + windowSize/2 - frameBorder, z + halfDepth + 0.01f);
            glEnd();

            // Window division lines (for realism)
            if (windowSize > 0.6f) {  // Only for larger windows
                glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
                glLineWidth(2.0f);  // Thicker lines for visibility

                glBegin(GL_LINES);
                // Horizontal divider
                glVertex3f(windowX - windowSize/2 + frameBorder, windowY, z + halfDepth + 0.011f);
                glVertex3f(windowX + windowSize/2 - frameBorder, windowY, z + halfDepth + 0.011f);

                // Vertical divider
                glVertex3f(windowX, windowY - windowSize/2 + frameBorder, z + halfDepth + 0.011f);
                glVertex3f(windowX, windowY + windowSize/2 - frameBorder, z + halfDepth + 0.011f);
                glEnd();
            }

            // Now switch blend mode for the glow effect
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);

            // Add enhanced window glow with clearer definition
            float glowIntensity = windowPulse * 0.6f;

            // Store glow color components in separate variables
            float r, g, b;
            if (windowType < 5) { // Blue windows
                r = 0.0f; g = 0.5f; b = 1.0f;
            }
            else if (windowType < 8) { // Yellow windows
                r = 1.0f; g = 0.9f; b = 0.3f;
            }
            else if (windowType == 8) { // Red windows
                r = 1.0f; g = 0.1f; b = 0.1f;
                glowIntensity = glowIntensity * ((sin(time * 15.0f) > 0) ? 1.0f : 0.2f);
            }
            else { // Purple windows
                r = 1.0f; g = 0.0f; b = 0.8f;
            }

            // Draw glow with fading gradient (using multiple quads with decreasing opacity)
            for (int g = 0; g < 3; g++) {
                float gScale = 1.0f + g * 0.5f;  // Increase size for each layer
                float gAlpha = glowIntensity * (1.0f - g * 0.3f);  // Decrease alpha for outer layers

                glColor4f(r, g, b, gAlpha);
                glBegin(GL_QUADS);
                glVertex3f(windowX - (windowSize * gScale)/2, windowY - (windowSize * gScale)/2, z + halfDepth + 0.012f);
                glVertex3f(windowX + (windowSize * gScale)/2, windowY - (windowSize * gScale)/2, z + halfDepth + 0.012f);
                glVertex3f(windowX + (windowSize * gScale)/2, windowY + (windowSize * gScale)/2, z + halfDepth + 0.012f);
                glVertex3f(windowX - (windowSize * gScale)/2, windowY + (windowSize * gScale)/2, z + halfDepth + 0.012f);
                glEnd();
            }
        }
    }

    // Side windows with same improved visuals
    if (depth > 2.0f) {
        for (int i = 0; i < numWindowsVertical; i += 2) {
            for (int j = 0; j < 3; j++) {
                if (rand() % 3 == 0) continue;

                float windowY = 2.0f + i * 2.0f;
                if (windowY >= height) continue;

                float windowZ = z - halfDepth + j * (depth / 2.0f);
                float windowSize = 0.5f + static_cast<float>(rand() % 3) * 0.1f;
                float frameBorder = 0.05f;

                int windowType = rand() % 10;

                // Right side windows
                // Draw window frame first (black border)
                glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
                glBegin(GL_QUADS);
                glVertex3f(x + halfWidth + 0.005f, windowY - windowSize/2, windowZ - windowSize/2);
                glVertex3f(x + halfWidth + 0.005f, windowY - windowSize/2, windowZ + windowSize/2);
                glVertex3f(x + halfWidth + 0.005f, windowY + windowSize/2, windowZ + windowSize/2);
                glVertex3f(x + halfWidth + 0.005f, windowY + windowSize/2, windowZ - windowSize/2);
                glEnd();

                // Draw window inner with appropriate color
                if (windowType < 5) {
                    float blueIntensity = windowPulse * (0.8f + 0.2f * sinf(time * 3.0f + i * j * 0.1f));
                    glColor4f(0.0f, 0.7f * blueIntensity, 1.0f * blueIntensity, 0.95f);
                }
                else if (windowType < 8) {
                    float yellowIntensity = 0.5f + 0.5f * sinf(time * 2.0f + i * 0.1f + j * 0.2f);
                    glColor4f(1.0f * yellowIntensity, 0.9f * yellowIntensity, 0.5f * yellowIntensity, 0.95f);
                }
                else if (windowType == 8) {
                    float redFlicker = (sin(time * 15.0f) > 0) ? 1.0f : 0.2f;
                    glColor4f(1.0f * redFlicker, 0.1f * redFlicker, 0.1f * redFlicker, 0.95f);
                }
                else {
                    glColor4f(1.0f * windowPulse, 0.0f, 0.8f * windowPulse, 0.95f);
                }

                glBegin(GL_QUADS);
                glVertex3f(x + halfWidth + 0.01f, windowY - windowSize/2 + frameBorder, windowZ - windowSize/2 + frameBorder);
                glVertex3f(x + halfWidth + 0.01f, windowY - windowSize/2 + frameBorder, windowZ + windowSize/2 - frameBorder);
                glVertex3f(x + halfWidth + 0.01f, windowY + windowSize/2 - frameBorder, windowZ + windowSize/2 - frameBorder);
                glVertex3f(x + halfWidth + 0.01f, windowY + windowSize/2 - frameBorder, windowZ - windowSize/2 + frameBorder);
                glEnd();

                // Left side windows (same approach)
                glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
                glBegin(GL_QUADS);
                glVertex3f(x - halfWidth - 0.005f, windowY - windowSize/2, windowZ - windowSize/2);
                glVertex3f(x - halfWidth - 0.005f, windowY - windowSize/2, windowZ + windowSize/2);
                glVertex3f(x - halfWidth - 0.005f, windowY + windowSize/2, windowZ + windowSize/2);
                glVertex3f(x - halfWidth - 0.005f, windowY + windowSize/2, windowZ - windowSize/2);
                glEnd();

                // Draw window inner with appropriate color (same as right side)
                if (windowType < 5) {
                    float blueIntensity = windowPulse * (0.8f + 0.2f * sinf(time * 3.0f + i * j * 0.1f));
                    glColor4f(0.0f, 0.7f * blueIntensity, 1.0f * blueIntensity, 0.95f);
                }
                else if (windowType < 8) {
                    float yellowIntensity = 0.5f + 0.5f * sinf(time * 2.0f + i * 0.1f + j * 0.2f);
                    glColor4f(1.0f * yellowIntensity, 0.9f * yellowIntensity, 0.5f * yellowIntensity, 0.95f);
                }
                else if (windowType == 8) {
                    float redFlicker = (sin(time * 15.0f) > 0) ? 1.0f : 0.2f;
                    glColor4f(1.0f * redFlicker, 0.1f * redFlicker, 0.1f * redFlicker, 0.95f);
                }
                else {
                    glColor4f(1.0f * windowPulse, 0.0f, 0.8f * windowPulse, 0.95f);
                }

                glBegin(GL_QUADS);
                glVertex3f(x - halfWidth - 0.01f, windowY - windowSize/2 + frameBorder, windowZ - windowSize/2 + frameBorder);
                glVertex3f(x - halfWidth - 0.01f, windowY - windowSize/2 + frameBorder, windowZ + windowSize/2 - frameBorder);
                glVertex3f(x - halfWidth - 0.01f, windowY + windowSize/2 - frameBorder, windowZ + windowSize/2 - frameBorder);
                glVertex3f(x - halfWidth - 0.01f, windowY + windowSize/2 - frameBorder, windowZ - windowSize/2 + frameBorder);
                glEnd();
            }
        }
    }

    glDisable(GL_BLEND);
}
}

void drawGrid(float size, int divisions) {
    float step = size / divisions;
    float halfSize = size / 2.0f;
    float startY = 0.0f;
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Draw the grid surface with a subtle glow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glColor4f(0.05f, 0.0f, 0.1f, 0.7f); // Dark purple with alpha
    glBegin(GL_QUADS);
    glVertex3f(-halfSize, startY - 0.01f, -halfSize);
    glVertex3f(halfSize, startY - 0.01f, -halfSize);
    glVertex3f(halfSize, startY - 0.01f, halfSize);
    glVertex3f(-halfSize, startY - 0.01f, halfSize);
    glEnd();

    // Apply grid offset for animation with some acceleration/deceleration
    float offsetZ = fmodf(gridOffset * step, step);

    // Variable speed grid - speeds up and slows down
    float speedFactor = 1.0f + 0.5f * sinf(time * 0.3f);
    offsetZ *= speedFactor;

    // Draw grid lines along Z axis (pink)
    for (int i = 0; i <= divisions; i++) {
        float x = -halfSize + i * step;
        float thickness = 0.1f;

        if (i % 5 == 0) {
            thickness = 0.3f; // Make every 5th line thicker
            glColor4f(0.9f, 0.2f, 0.8f, 0.9f); // Solid pink for major lines
        } else {
            // Pulsating effect for minor lines
            float pulse = 0.5f + 0.5f * sinf(time * 2.0f + i * 0.1f);
            glColor4f(0.9f, 0.2f, 0.8f, 0.5f + 0.3f * pulse);
        }

        glBegin(GL_QUADS);
        glVertex3f(x - thickness/2, startY, -halfSize + offsetZ);
        glVertex3f(x + thickness/2, startY, -halfSize + offsetZ);
        glVertex3f(x + thickness/2, startY, halfSize + offsetZ);
        glVertex3f(x - thickness/2, startY, halfSize + offsetZ);
        glEnd();

        // Add vertical "light beams" on major grid lines
        if (i % 5 == 0 && i % 10 != 0) { // Only on some major lines
            float beamHeight = 10.0f + 5.0f * sinf(time + i);
            float beamAlpha = 0.2f + 0.1f * sinf(time * 2.0f + i);

            glColor4f(0.9f, 0.2f, 0.8f, beamAlpha); // Pink with alpha

            glBegin(GL_QUADS);
            glVertex3f(x - thickness, startY, -halfSize + i * step + offsetZ);
            glVertex3f(x + thickness, startY, -halfSize + i * step + offsetZ);
            glVertex3f(x + thickness/3, beamHeight, -halfSize + i * step + offsetZ);
            glVertex3f(x - thickness/3, beamHeight, -halfSize + i * step + offsetZ);
            glEnd();
        }
    }

    // Draw grid lines along X axis (blue) with enhanced effects
    for (int i = 0; i <= divisions; i++) {
        float z = -halfSize + i * step + offsetZ;
        if (z > halfSize) continue;

        float thickness = 0.1f;
        if (i % 5 == 0) {
            thickness = 0.3f; // Make every 5th line thicker
            glColor4f(0.0f, 0.7f, 1.0f, 0.9f); // Solid blue for major lines
        } else {
            // Pulsating effect with phase shift
            float pulse = 0.5f + 0.5f * sinf(time * 2.0f + i * 0.1f + 1.5f); // Phase shifted from pink
            glColor4f(0.0f, 0.7f, 1.0f, 0.5f + 0.3f * pulse);
        }

        glBegin(GL_QUADS);
        glVertex3f(-halfSize, startY, z - thickness/2);
        glVertex3f(halfSize, startY, z - thickness/2);
        glVertex3f(halfSize, startY, z + thickness/2);
        glVertex3f(-halfSize, startY, z + thickness/2);
        glEnd();
    }

    // Add "data transfer" pulses moving along the grid lines
    int numPulses = 5;
    for (int i = 0; i < numPulses; i++) {
        // Calculate pulse position (moves from back to front)
        float pulsePhase = fmodf(time * 0.5f + i * 0.2f, 1.0f);
        float pulseZ = -halfSize + pulsePhase * size;

        // Pulse size and alpha (gets smaller as it approaches viewer)
        float pulseSize = 1.0f - pulsePhase * 0.7f;
        float pulseAlpha = 0.7f - pulsePhase * 0.5f;

        // Alternate between blue and pink pulses
        if (i % 2 == 0) {
            glColor4f(0.0f, 0.7f, 1.0f, pulseAlpha); // Blue
        } else {
            glColor4f(0.9f, 0.2f, 0.8f, pulseAlpha); // Pink
        }

        // Draw pulses at intersections of major grid lines
        for (int j = -4; j <= 4; j += 2) {
            float pulseX = j * (size / 10.0f); // Position on major grid lines

            glBegin(GL_QUADS);
            glVertex3f(pulseX - pulseSize, startY + 0.1f, pulseZ - pulseSize);
            glVertex3f(pulseX + pulseSize, startY + 0.1f, pulseZ - pulseSize);
            glVertex3f(pulseX + pulseSize, startY + 0.1f, pulseZ + pulseSize);
            glVertex3f(pulseX - pulseSize, startY + 0.1f, pulseZ + pulseSize);
            glEnd();
        }
    }

    glDisable(GL_BLEND);
}

void drawVortex(float radius, int segments, int rings) {
    float ringStep = radius / rings;
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Move vortex to the left above buildings
    glPushMatrix();
    glTranslatef(-20.0f, 20.0f, -50.0f); // Left above buildings
    glRotatef(vortexAngle, 0.0f, 1.0f, 0.0f);

    for (int r = 0; r < rings; r++) {
        float innerRadius = r * ringStep;
        float outerRadius = (r + 1) * ringStep;
        float heightFactor = 0.5f + 0.2f * sinf(time * 2.0f + r * 0.5f);
        float heightInner = r * heightFactor;
        float heightOuter = (r + 1) * heightFactor;

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
            float pulse = 0.7f + 0.3f * sinf(time * 3.0f + r * 0.5f + i * 0.1f);

            if (i % 2 == 0) {
                glColor4f(0.9f * pulse, 0.2f * pulse, 0.8f * pulse, 0.8f); // Pulsating pink
            } else {
                glColor4f(0.0f, 0.7f * pulse, 1.0f * pulse, 0.8f); // Pulsating blue
            }

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

    // Add bobbing animation to cars
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float verticalOffset = sinf(time * 4.0f + x) * 0.1f;

    // Choose car color with pulsating effect
    float pulseIntensity = 0.15f * sinf(time * 3.0f) + 0.85f;

    if (car.isBlue) {
        glColor3f(0.0f, 0.7f * pulseIntensity, 1.0f * pulseIntensity); // Blue with pulse
    } else {
        glColor3f(0.9f * pulseIntensity, 0.2f * pulseIntensity, 0.8f * pulseIntensity); // Pink with pulse
    }

    // Car body - using a display list would be more efficient but we'll do direct rendering here
    glPushMatrix();
    glTranslatef(x, 0.5f + verticalOffset, z);

    // Bottom
    glBegin(GL_QUADS);
    glVertex3f(-carWidth/2, 0.0f, -carLength/2);
    glVertex3f(carWidth/2, 0.0f, -carLength/2);
    glVertex3f(carWidth/2, 0.0f, carLength/2);
    glVertex3f(-carWidth/2, 0.0f, carLength/2);
    glEnd();

    // Top
    glBegin(GL_QUADS);
    glVertex3f(-carWidth/2, carHeight, -carLength/2);
    glVertex3f(carWidth/2, carHeight, -carLength/2);
    glVertex3f(carWidth/2, carHeight, carLength/2);
    glVertex3f(-carWidth/2, carHeight, -carLength/2);
    glEnd();

    // Front - more aerodynamic shape
    glBegin(GL_QUADS);
    glVertex3f(-carWidth/2, 0.0f, carLength/2);
    glVertex3f(carWidth/2, 0.0f, carLength/2);
    glVertex3f(carWidth/2, carHeight * 0.8f, carLength/2 - 0.3f);
    glVertex3f(-carWidth/2, carHeight * 0.8f, carLength/2 - 0.3f);

    // Additional front hood slant
    glVertex3f(-carWidth/2, carHeight * 0.8f, carLength/2 - 0.3f);
    glVertex3f(carWidth/2, carHeight * 0.8f, carLength/2 - 0.3f);
    glVertex3f(carWidth/2, carHeight, carLength/2 - 1.0f);
    glVertex3f(-carWidth/2, carHeight, carLength/2 - 1.0f);
    glEnd();

    // Back - angled for sporty look
    glBegin(GL_QUADS);
    glVertex3f(-carWidth/2, 0.0f, -carLength/2);
    glVertex3f(carWidth/2, 0.0f, -carLength/2);
    glVertex3f(carWidth/2, carHeight * 0.9f, -carLength/2 + 0.4f);
    glVertex3f(-carWidth/2, carHeight * 0.9f, -carLength/2 + 0.4f);
    glEnd();

    // Left side
    glBegin(GL_QUADS);
    glVertex3f(-carWidth/2, 0.0f, -carLength/2);
    glVertex3f(-carWidth/2, 0.0f, carLength/2);
    glVertex3f(-carWidth/2, carHeight, carLength/2);
    glVertex3f(-carWidth/2, carHeight, -carLength/2);
    glEnd();

    // Right side
    glBegin(GL_QUADS);
    glVertex3f(carWidth/2, 0.0f, -carLength/2);
    glVertex3f(carWidth/2, 0.0f, carLength/2);
    glVertex3f(carWidth/2, carHeight, carLength/2);
    glVertex3f(carWidth/2, carHeight, -carLength/2);
    glEnd();

    // Windows (black with slight reflection)
    glColor3f(0.1f, 0.1f, 0.3f);

    // Front windshield
    glBegin(GL_QUADS);
    glVertex3f(-carWidth/2 + 0.2f, carHeight*0.6f, carLength/2 - 0.3f);
    glVertex3f(carWidth/2 - 0.2f, carHeight*0.6f, carLength/2 - 0.3f);
    glVertex3f(carWidth/2 - 0.2f, carHeight - 0.1f, carLength/2 - 1.0f);
    glVertex3f(-carWidth/2 + 0.2f, carHeight - 0.1f, carLength/2 - 1.0f);
    glEnd();

    // Rear windshield
    glBegin(GL_QUADS);
    glVertex3f(-carWidth/2 + 0.2f, carHeight*0.6f, -carLength/2 + 0.2f);
    glVertex3f(carWidth/2 - 0.2f, carHeight*0.6f, -carLength/2 + 0.2f);
    glVertex3f(carWidth/2 - 0.2f, carHeight - 0.1f, -carLength/2 + 0.7f);
    glVertex3f(-carWidth/2 + 0.2f, carHeight - 0.1f, -carLength/2 + 0.7f);
    glEnd();

    // Headlights with glowing effect (bright white)
    float headlightPulse = 0.7f + 0.3f * sinf(time * 10.0f); // Fast pulsing for headlights
    glColor3f(1.0f, 1.0f, headlightPulse);

    glBegin(GL_QUADS);
    // Left headlight
    glVertex3f(-carWidth/2 + 0.2f, carHeight*0.3f, carLength/2 + 0.01f);
    glVertex3f(-carWidth/2 + 0.6f, carHeight*0.3f, carLength/2 + 0.01f);
    glVertex3f(-carWidth/2 + 0.6f, carHeight*0.5f, carLength/2 + 0.01f);
    glVertex3f(-carWidth/2 + 0.2f, carHeight*0.5f, carLength/2 + 0.01f);

    // Right headlight
    glVertex3f(carWidth/2 - 0.2f, carHeight*0.3f, carLength/2 + 0.01f);
    glVertex3f(carWidth/2 - 0.6f, carHeight*0.3f, carLength/2 + 0.01f);
    glVertex3f(carWidth/2 - 0.6f, carHeight*0.5f, carLength/2 + 0.01f);
    glVertex3f(carWidth/2 - 0.2f, carHeight*0.5f, carLength/2 + 0.01f);
    glEnd();

    // Add headlight beams
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    if (car.isBlue) {
        glColor4f(0.0f, 0.7f, 1.0f, 0.3f); // Blue glow
    } else {
        glColor4f(0.9f, 0.2f, 0.8f, 0.3f); // Pink glow
    }

    glBegin(GL_TRIANGLES);
    // Left headlight beam
    glVertex3f(-carWidth/2 + 0.4f, carHeight*0.4f, carLength/2 + 0.01f);
    glVertex3f(-carWidth/2 + 0.2f, carHeight*0.3f, carLength/2 + 5.0f);
    glVertex3f(-carWidth/2 + 0.6f, carHeight*0.5f, carLength/2 + 5.0f);

    // Right headlight beam
    glVertex3f(carWidth/2 - 0.4f, carHeight*0.4f, carLength/2 + 0.01f);
    glVertex3f(carWidth/2 - 0.2f, carHeight*0.3f, carLength/2 + 5.0f);
    glVertex3f(carWidth/2 - 0.6f, carHeight*0.5f, carLength/2 + 5.0f);
    glEnd();

    // Tail lights (red with pulsing brake effect)
    float tailLightPulse = 0.6f + 0.4f * sinf(time * 5.0f); // Slower pulsing for brake lights
    glColor3f(1.0f * tailLightPulse, 0.1f, 0.1f);

    glBegin(GL_QUADS);
    // Left tail light
    glVertex3f(-carWidth/2 + 0.2f, carHeight*0.3f, -carLength/2 + 0.01f);
    glVertex3f(-carWidth/2 + 0.6f, carHeight*0.3f, -carLength/2 + 0.01f);
    glVertex3f(-carWidth/2 + 0.6f, carHeight*0.7f, -carLength/2 + 0.01f);
    glVertex3f(-carWidth/2 + 0.2f, carHeight*0.7f, -carLength/2 + 0.01f);

    // Right tail light
    glVertex3f(carWidth/2 - 0.2f, carHeight*0.3f, -carLength/2 + 0.01f);
    glVertex3f(carWidth/2 - 0.6f, carHeight*0.3f, -carLength/2 + 0.01f);
    glVertex3f(carWidth/2 - 0.6f, carHeight*0.7f, -carLength/2 + 0.01f);
    glVertex3f(carWidth/2 - 0.2f, carHeight*0.7f, -carLength/2 + 0.01f);
    glEnd();

    // Add wheels
    glColor3f(0.1f, 0.1f, 0.1f); // Black wheels

    float wheelRadius = 0.5f;
    float wheelWidth = 0.3f;

    // Draw 4 wheels
    for (int i = 0; i < 4; i++) {
        float wheelX = (i % 2 == 0) ? -carWidth/2 - wheelWidth/2 : carWidth/2 + wheelWidth/2;
        float wheelZ = (i < 2) ? carLength/2 - wheelRadius : -carLength/2 + wheelRadius;

        // Spinning animation for wheels
        float wheelRotation = fmodf(time * 720.0f, 360.0f); // Fast spinning

        glPushMatrix();
        glTranslatef(wheelX, wheelRadius, wheelZ);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f); // Orient the wheel correctly
        glRotatef(wheelRotation, 0.0f, 0.0f, 1.0f); // Spin the wheel

        // Draw wheel as a cylinder with caps
        GLUquadricObj *quadric = gluNewQuadric();
        gluCylinder(quadric, wheelRadius, wheelRadius, wheelWidth, 16, 1);

        // Draw wheel caps
        gluDisk(quadric, 0.0f, wheelRadius, 16, 1);
        glTranslatef(0.0f, 0.0f, wheelWidth);
        gluDisk(quadric, 0.0f, wheelRadius, 16, 1);

        gluDeleteQuadric(quadric);
        glPopMatrix();
    }

    // Enhanced under-car glow with particle effect
    // Pulsating underglow
       // Enhanced under-car glow with particle effect
    // Pulsating underglow
    float glowPulse = 0.7f + 0.3f * sinf(time * 2.0f);

    if (car.isBlue) {
        glColor4f(0.0f, 0.7f * glowPulse, 1.0f * glowPulse, 0.5f); // Blue glow
    } else {
        glColor4f(0.9f * glowPulse, 0.2f * glowPulse, 0.8f * glowPulse, 0.5f); // Pink glow
    }

    // Main underglow rectangle
    glBegin(GL_QUADS);
    glVertex3f(-carWidth/2 - 0.3f, -0.3f, -carLength/2 - 0.3f);
    glVertex3f(carWidth/2 + 0.3f, -0.3f, -carLength/2 - 0.3f);
    glVertex3f(carWidth/2 + 0.3f, -0.3f, carLength/2 + 0.3f);
    glVertex3f(-carWidth/2 - 0.3f, -0.3f, carLength/2 + 0.3f);
    glEnd();

    // Add particles falling from the car
    srand(static_cast<unsigned int>(car.x * 1000.0f + car.z * 100.0f));
    glPointSize(3.0f);
    glBegin(GL_POINTS);

    for (int i = 0; i < 20; i++) {
        float particleOffset = static_cast<float>(rand()) / RAND_MAX * carLength - carLength/2;
        float particleX = -carWidth/2 + static_cast<float>(rand()) / RAND_MAX * carWidth * 2.0f;
        float particleLifetime = fmodf(time * 3.0f + i * 0.1f, 1.0f);
        float particleY = -0.4f - particleLifetime * 0.5f;

        // Fade out as particles fall
        if (car.isBlue) {
            glColor4f(0.0f, 0.7f, 1.0f, 1.0f - particleLifetime);
        } else {
            glColor4f(0.9f, 0.2f, 0.8f, 1.0f - particleLifetime);
        }

        glVertex3f(particleX, particleY, particleOffset);
    }

    glEnd();
    glDisable(GL_BLEND);

    glPopMatrix();
}

void drawMoon() {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    glPushMatrix();

    // Position the moon on the right above buildings
    glTranslatef(20.0f, 25.0f, -50.0f);

    glDisable(GL_LIGHTING);

    // Enhanced moon with detailed surface
    glColor3f(0.95f, 0.95f, 1.0f);
    GLUquadricObj *moon = gluNewQuadric();
    gluQuadricTexture(moon, GL_TRUE);
    gluSphere(moon, 4.0f, 30, 30); // Larger and more detailed moon

    // Enhanced moon glow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Inner glow (bright)
    glColor4f(0.9f, 0.9f, 1.0f, 0.5f);
    gluSphere(moon, 4.2f, 30, 30);

    // Multiple layers of fading outer glow
    for (float i = 0.0f; i < 1.0f; i += 0.2f) {
        float alpha = 0.4f * (1.0f - i);
        float pulseSize = 1.0f + 0.1f * sinf(time * 1.0f + i * 3.0f);
        glColor4f(0.9f, 0.9f, 1.0f, alpha);
        gluSphere(moon, 4.5f + i * 3.0f * pulseSize, 30, 30);
    }

    // Moon craters with more detail
    glColor3f(0.85f, 0.85f, 0.9f);

    // Add 8-10 different sized craters in various positions
    for (int i = 0; i < 10; i++) {
        glPushMatrix();
        // Use consistent random positions based on index
        srand(i * 123);
        float craterX = -3.0f + static_cast<float>(rand()) / RAND_MAX * 6.0f;
        float craterY = -3.0f + static_cast<float>(rand()) / RAND_MAX * 6.0f;
        float craterZ = -1.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
        float craterSize = 0.3f + static_cast<float>(rand()) / RAND_MAX * 0.8f;

        glTranslatef(craterX, craterY, craterZ);
        gluSphere(moon, craterSize, 10, 10);
        glPopMatrix();
    }

    gluDeleteQuadric(moon);

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

void drawGalaxies() {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_LIGHTING);

    for (size_t i = 0; i < galaxies.size(); i++) {
        const Galaxy& galaxy = galaxies[i];

        glPushMatrix();
        glTranslatef(galaxy.x, galaxy.y, galaxy.z);
        glRotatef(galaxy.rotation + time * 5.0f, 0.0f, 1.0f, 0.0f);

        if (galaxy.type == 0) { // Spiral galaxy
            // Draw spiral arms
            float armWidth = galaxy.size * 0.2f;

            for (int arm = 0; arm < 2; arm++) {
                float armRotation = arm * 180.0f; // Rotate arms 180 degrees apart

                for (float t = 0.0f; t < 6.0f; t += 0.1f) {
                    float r = t * galaxy.size / 6.0f;
                    float theta = t * 2.5f + armRotation;
                    float x = r * cosf(theta);
                    float z = r * sinf(theta);

                    // Color based on distance from center
                    float blueIntensity = (6.0f - t) / 6.0f;
                    float redIntensity = t / 6.0f;
                    float brightness = 0.7f + 0.3f * sinf(time + t);

                    glColor4f(0.6f * redIntensity * brightness,
                              0.4f * brightness,
                              0.8f * blueIntensity * brightness,
                              0.3f * (1.0f - t/6.0f) * brightness);

                    glPointSize(armWidth * (1.0f - t/6.0f) + 1.0f);
                    glBegin(GL_POINTS);
                    glVertex3f(x, 0.0f, z);
                    glEnd();

                    // Draw some additional stars around the arm
                    glPointSize(1.0f);
                    glBegin(GL_POINTS);
                    for (int j = 0; j < 3; j++) {
                        float offsetX = x + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * armWidth;
                        float offsetZ = z + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * armWidth;
                        glVertex3f(offsetX, 0.0f, offsetZ);
                    }
                    glEnd();
                }
            }

            // Galaxy core
            glColor4f(1.0f, 0.9f, 0.7f, 0.7f);
            GLUquadricObj *core = gluNewQuadric();
            gluSphere(core, galaxy.size * 0.15f, 16, 16);
            gluDeleteQuadric(core);

            // Core glow
            glColor4f(0.8f, 0.7f, 1.0f, 0.3f);
            GLUquadricObj *glow = gluNewQuadric();
            gluSphere(glow, galaxy.size * 0.25f, 16, 16);
            gluDeleteQuadric(glow);
        }
        else { // Elliptical galaxy
            // Draw as a fuzzy ellipsoid
            glScalef(1.0f, 0.4f, 1.0f); // Flatten it

            // Draw layers of decreasing opacity
            for (float r = 0.0f; r < 1.0f; r += 0.2f) {
                float size = galaxy.size * (1.0f - r);
                float alpha = 0.3f * (1.0f - r);

                glColor4f(0.8f, 0.7f, 1.0f, alpha);
                GLUquadricObj *layer = gluNewQuadric();
                gluSphere(layer, size, 16, 16);
                gluDeleteQuadric(layer);
            }

            // Add some random stars within the elliptical galaxy
            glPointSize(2.0f);
            glBegin(GL_POINTS);
            for (int j = 0; j < 100; j++) {
                float phi = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
                float theta = static_cast<float>(rand()) / RAND_MAX * M_PI;
                float r = static_cast<float>(rand()) / RAND_MAX * galaxy.size * 0.8f;

                float x = r * sinf(theta) * cosf(phi);
                float y = r * sinf(theta) * sinf(phi) * 0.4f; // Flatten
                float z = r * cosf(theta);

                float brightness = static_cast<float>(rand()) / RAND_MAX;
                glColor4f(0.7f + 0.3f * brightness,
                          0.7f + 0.3f * brightness,
                          1.0f,
                          0.7f * brightness);

                glVertex3f(x, y, z);
            }
            glEnd();
        }

        glPopMatrix();
    }

    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
}

void drawSky() {
    // Set dark blue/purple sky color (no day/night cycle - always night)
    glClearColor(0.02f, 0.0f, 0.05f, 1.0f);

    // Draw galaxies first (as background)
    drawGalaxies();

    // Draw stars with enhanced visuals
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (size_t i = 0; i < stars.size(); i++) {
        const Star& star = stars[i];

        // Different twinkling patterns for different stars
        float twinkleSpeed = 3.0f + (i % 5) * 1.0f;
        float twinklePhase = i * 0.1f;
        float twinkle = 0.5f + 0.5f * sinf(time * twinkleSpeed + twinklePhase);
        float brightness = star.brightness * twinkle;

        // Variable star size
        glPointSize(star.size * (0.8f + 0.4f * twinkle));

        // Different star colors
        if (star.colorType < 7) { // White/blue stars (most common)
            glColor3f(0.8f + 0.2f * twinkle,
                      0.8f + 0.2f * twinkle,
                      1.0f);
        }
        else if (star.colorType < 9) { // Yellow/orange stars
            glColor3f(1.0f,
                      0.7f + 0.3f * twinkle,
                      0.4f * twinkle);
        }
        else { // Red giants (rare)
            glColor3f(1.0f,
                      0.3f * twinkle,
                      0.2f * twinkle);
        }

        glBegin(GL_POINTS);
        glVertex3f(star.x, star.y, star.z);
        glEnd();

        // Add glow for brighter stars
        if (star.brightness > 0.8f) {
            float glowSize = star.size * 3.0f * twinkle;

            // Set glow color (slightly match star color)
            if (star.colorType < 7) { // White/blue stars
                glColor4f(0.6f, 0.6f, 1.0f, 0.2f * brightness);
            }
            else if (star.colorType < 9) { // Yellow/orange stars
                glColor4f(1.0f, 0.7f, 0.3f, 0.2f * brightness);
            }
            else { // Red giants
                glColor4f(1.0f, 0.3f, 0.2f, 0.2f * brightness);
            }

            // Draw glow as a point with larger size
            glPointSize(glowSize);
            glBegin(GL_POINTS);
            glVertex3f(star.x, star.y, star.z);
            glEnd();

            // For a few super bright stars, add lens flare effect
            if (star.brightness > 0.95f && i % 10 == 0) {
                float flareIntensity = 0.7f * twinkle;

                // Draw horizontal flare line
                glBegin(GL_LINES);
                glColor4f(1.0f, 1.0f, 1.0f, 0.3f * flareIntensity);
                glVertex3f(star.x - glowSize, star.y, star.z);
                glVertex3f(star.x + glowSize, star.y, star.z);
                glEnd();

                // Draw vertical flare line
                glBegin(GL_LINES);
                glColor4f(1.0f, 1.0f, 1.0f, 0.3f * flareIntensity);
                glVertex3f(star.x, star.y - glowSize, star.z);
                glVertex3f(star.x, star.y + glowSize, star.z);
                glEnd();
            }
        }
    }

    // Draw enhanced moon last (in foreground)
    drawMoon();

    glDisable(GL_BLEND);
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
        snprintf(title, sizeof(title), "Enhanced Retrowave Landscape - FPS: %.1f", fps);
        glutSetWindowTitle(title);
    }
}
