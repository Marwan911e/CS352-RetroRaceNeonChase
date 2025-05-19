#include <windows.h>
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <ctime>
#include <string>
#include <iostream>
#include <cstdio>

// Window dimensions
const int SCR_WIDTH = 1200;
const int SCR_HEIGHT = 800;

// Camera variables
float cameraX = 0.0f, cameraY = 10.0f, cameraZ = 60.0f;
float lookX = 0.0f, lookY = 0.0f, lookZ = -1.0f;

// Animation variables
float gridOffset = 0.0f;
float vortexAngle = 0.0f;
float tunnelDepth = 0.0f;
float lastTime = 0.0f;
float buildingPulse = 0.0f;
bool showMusicVisualization = true;

// Object structures
struct Building {
    float x, z, width, height, depth;
};

struct Star {
    float x, y, z, brightness, size;
    int colorType;
};

struct Spinner {
    float x, y, z;
    float radius, rotation;
    float rotationSpeed;
    int type; // 0=circular, 1=spiral
    bool isPink; // true=pink, false=blue
};

struct Car {
    float x, z;
    bool isBlue;
    float speed;
};

// Collections
std::vector<Building> buildings;
std::vector<Star> stars;
std::vector<Spinner> spinners;
std::vector<Car> cars;

// Music player (Windows-native)
class SimpleAudioPlayer {
private:
    bool isPlaying;
    float volume;
    std::string currentFile;

public:
    SimpleAudioPlayer() : isPlaying(false), volume(0.5f) {}

    bool playMusic(const std::string& filename) {
        stopMusic();
        std::string openCommand = "open \"" + filename + "\" type mpegvideo alias retroMusic";
        if (mciSendStringA(openCommand.c_str(), NULL, 0, NULL) != 0) {
            std::cerr << "Failed to open MP3 file: " << filename << std::endl;
            return false;
        }

        setVolume(volume);
        std::string playCommand = "play retroMusic repeat";
        if (mciSendStringA(playCommand.c_str(), NULL, 0, NULL) != 0) {
            std::cerr << "Failed to play MP3 file" << std::endl;
            mciSendStringA("close retroMusic", NULL, 0, NULL);
            return false;
        }

        currentFile = filename;
        isPlaying = true;
        return true;
    }

    void toggleMusic() {
        if (isPlaying) {
            mciSendStringA("pause retroMusic", NULL, 0, NULL);
            isPlaying = false;
        } else {
            mciSendStringA("resume retroMusic", NULL, 0, NULL);
            isPlaying = true;
        }
    }

    void stopMusic() {
        if (isPlaying) {
            mciSendStringA("stop retroMusic", NULL, 0, NULL);
            mciSendStringA("close retroMusic", NULL, 0, NULL);
            isPlaying = false;
        }
    }

    void setVolume(float vol) {
        volume = (vol < 0.0f) ? 0.0f : (vol > 1.0f) ? 1.0f : vol;
        int winVolume = static_cast<int>(volume * 1000);
        char buffer[50];
        sprintf(buffer, "%d", winVolume);
        std::string volCommand = "setaudio retroMusic volume to ";
        volCommand += buffer;
        mciSendStringA(volCommand.c_str(), NULL, 0, NULL);
    }

    void adjustVolume(float change) {
        setVolume(volume + change);
        std::cout << "Volume: " << (volume * 100) << "%" << std::endl;
    }

    ~SimpleAudioPlayer() {
        stopMusic();
    }
};

// Global audio player
SimpleAudioPlayer audioPlayer;

// FPS calculation
int frameCount = 0;
float fps = 0.0f;
float currentTime = 0.0f;
float previousTime = 0.0f;

// Function prototypes
void init();
void display();
void reshape(int width, int height);
void timer(int value);
void keyboard(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void drawBuilding(const Building& building);
void drawGrid(float size, int divisions);
void drawSpinners();
void drawSpinner(const Spinner& spinner, float time);
void drawTunnel(float radius, int segments, int rings);
void drawCar(const Car& car);
void drawSky();
void calculateFPS();
void initAudio();
void cleanup();
// Added new function prototypes for the shapes
void drawPyramid(float time);
void drawTorus(float time);

// Retro wave color palette (use consistently throughout)
struct RetroColor {
    static void Pink(float time, float alpha = 1.0f) {
        float pulse = 0.7f + 0.3f * sinf(time * 2.0f);
        glColor4f(1.0f * pulse, 0.1f * pulse, 0.8f * pulse, alpha);
    }

    static void Cyan(float time, float alpha = 1.0f) {
        float pulse = 0.7f + 0.3f * sinf(time * 2.0f);
        glColor4f(0.0f, 0.8f * pulse, 1.0f * pulse, alpha);
    }

    static void Gold(float time, float alpha = 1.0f) {
        float pulse = 0.7f + 0.3f * sinf(time * 2.0f);
        glColor4f(1.0f * pulse, 0.8f * pulse, 0.0f, alpha);
    }

    static void Purple(float time, float alpha = 1.0f) {
        float pulse = 0.7f + 0.3f * sinf(time * 2.0f);
        glColor4f(0.6f * pulse, 0.0f, 1.0f * pulse, alpha);
    }

    static void getPinkMaterial(float time, float alpha, GLfloat* color) {
        float pulse = 0.7f + 0.3f * sinf(time * 2.0f);
        color[0] = 1.0f * pulse;
        color[1] = 0.1f * pulse;
        color[2] = 0.8f * pulse;
        color[3] = alpha;
    }

    static void getCyanMaterial(float time, float alpha, GLfloat* color) {
        float pulse = 0.7f + 0.3f * sinf(time * 2.0f);
        color[0] = 0.0f;
        color[1] = 0.8f * pulse;
        color[2] = 1.0f * pulse;
        color[3] = alpha;
    }

    static void getGoldMaterial(float time, float alpha, GLfloat* color) {
        float pulse = 0.7f + 0.3f * sinf(time * 2.0f);
        color[0] = 1.0f * pulse;
        color[1] = 0.8f * pulse;
        color[2] = 0.0f;
        color[3] = alpha;
    }
};

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    glutCreateWindow("Retrowave City");

    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, timer, 0); // ~60 FPS

    // Initialize OpenGL
    init();

    // Initialize audio (play the background music)
    initAudio();

    // Register cleanup function
    atexit(cleanup);

    // Start main loop
    glutMainLoop();

    return 0;
}

void init() {
    // Set background color (deep purple)
    glClearColor(0.05f, 0.0f, 0.1f, 1.0f);

    // Enable depth testing and lighting
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Setup light
    GLfloat ambientLight[] = { 0.1f, 0.1f, 0.2f, 1.0f };
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 1.0f, 1.0f };
    GLfloat position[] = { -10.0f, 20.0f, 10.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    // Enable color material
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
    for (int i = 0; i < 200; i++) {
        Star s;
        s.x = -150.0f + static_cast<float>(rand()) / RAND_MAX * 300.0f;
        s.y = 20.0f + static_cast<float>(rand()) / RAND_MAX * 80.0f;
        s.z = -150.0f + static_cast<float>(rand()) / RAND_MAX * 100.0f;
        s.brightness = 0.5f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
        s.size = 1.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
        s.colorType = rand() % 10; // Different star colors
        stars.push_back(s);
    }

    // Initialize spinners
    // Main spinner (vortex tunnel in the sky)
    Spinner mainVortex;
    mainVortex.x = 0.0f;
    mainVortex.y = 30.0f;
    mainVortex.z = -80.0f;
    mainVortex.radius = 25.0f;
    mainVortex.rotation = 0.0f;
    mainVortex.rotationSpeed = 30.0f;
    mainVortex.type = 1; // spiral
    mainVortex.isPink = true;
    spinners.push_back(mainVortex);

    // Additional floating spinners
    for (int i = 0; i < 3; i++) {
        Spinner s;
        s.x = -40.0f + static_cast<float>(rand()) / RAND_MAX * 80.0f;
        s.y = 15.0f + static_cast<float>(rand()) / RAND_MAX * 20.0f;
        s.z = -100.0f + static_cast<float>(rand()) / RAND_MAX * 40.0f;
        s.radius = 3.0f + static_cast<float>(rand()) / RAND_MAX * 5.0f;
        s.rotation = 0.0f;
        s.rotationSpeed = 10.0f + static_cast<float>(rand()) / RAND_MAX * 30.0f;
        s.type = rand() % 2;
        s.isPink = (rand() % 2 == 0);
        spinners.push_back(s);
    }

    // Create cars
    for (int i = 0; i < 5; i++) {
        Car car;
        car.x = -8.0f + static_cast<float>(rand()) / RAND_MAX * 16.0f;
        car.z = -50.0f + static_cast<float>(rand()) / RAND_MAX * 60.0f;
        car.isBlue = (rand() % 2 == 0);
        car.speed = 15.0f + static_cast<float>(rand()) / RAND_MAX * 10.0f;
        cars.push_back(car);
    }

    // Initialize time
    previousTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Better graphics quality
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glShadeModel(GL_SMOOTH);
}

void initAudio() {
    // PLACE YOUR MP3 FILE IN THE SAME FOLDER AS YOUR EXE FILE
    std::string musicFile = "retrowave_music.mp3";

    if (!audioPlayer.playMusic(musicFile)) {
        std::cerr << "Warning: Failed to play background music." << std::endl;
    }
}

void cleanup() {
    audioPlayer.stopMusic();
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

    // Add the new shapes
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    drawPyramid(time);
    drawTorus(time);

    // Draw spinners (futuristic elements)
    drawSpinners();

    // Disable lighting temporarily for neon effects
    glDisable(GL_LIGHTING);

    // Draw tunnel effect in the sky
    drawTunnel(30.0f, 36, 15);

    // Draw grid
    drawGrid(100.0f, 40);

    // Draw buildings
    for (size_t i = 0; i < buildings.size(); i++) {
        drawBuilding(buildings[i]);
    }

    // Draw cars
    for (size_t i = 0; i < cars.size(); i++) {
        drawCar(cars[i]);
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

    // Set perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
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

    // Update grid animation
    gridOffset += 0.8f * deltaTime;
    if (gridOffset > 1.0f) gridOffset -= 1.0f;

    // Update vortex angle
    float vortexSpeed = 25.0f + 15.0f * sinf(currentTime * 0.2f);
    vortexAngle += vortexSpeed * deltaTime;
    if (vortexAngle > 360.0f) vortexAngle -= 360.0f;

    // Update tunnel depth
    tunnelDepth += 15.0f * deltaTime;
    if (tunnelDepth > 10.0f) tunnelDepth -= 10.0f;

    // Update spinner rotations
    for (size_t i = 0; i < spinners.size(); i++) {
        spinners[i].rotation += spinners[i].rotationSpeed * deltaTime;
        if (spinners[i].rotation > 360.0f) spinners[i].rotation -= 360.0f;
    }

    // Update car movement
    for (size_t i = 0; i < cars.size(); i++) {
        // Move cars with their individual speeds
        cars[i].z += cars[i].speed * deltaTime;

        // Reset position when car goes too far
        if (cars[i].z > 50.0f) {
            cars[i].z = -50.0f;
            cars[i].speed = 15.0f + static_cast<float>(rand()) / RAND_MAX * 10.0f;

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
        case 'w': // Move forward
            cameraX += lookX * speed;
            cameraZ += lookZ * speed;
            break;
        case 's': // Move backward
            cameraX -= lookX * speed;
            cameraZ -= lookZ * speed;
            break;
        case 'd': // Strafe left
            cameraX -= lookZ * speed;
            cameraZ += lookX * speed;
            break;
        case 'a': // Strafe right
            cameraX += lookZ * speed;
            cameraZ -= lookX * speed;
            break;
        case 'q': // Move up
            cameraY += speed;
            break;
        case 'e': // Move down
            cameraY -= speed;
            break;
        case 'p': // Toggle music playback
            audioPlayer.toggleMusic();
            break;
        case '+': // Increase volume
            audioPlayer.adjustVolume(0.1f);
            break;
        case '-': // Decrease volume
            audioPlayer.adjustVolume(-0.1f);
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
            {
                float tempX = lookX;
                lookX = lookX * cosf(rotationSpeed) - lookZ * sinf(rotationSpeed);
                lookZ = tempX * sinf(rotationSpeed) + lookZ * cosf(rotationSpeed);
            }
            break;
        case GLUT_KEY_RIGHT:
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

void drawBuilding(const Building& building) {
    float x = building.x;
    float z = building.z;
    float width = building.width;
    float height = building.height;
    float depth = building.depth;
    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;

    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float buildingOffset = sinf(time * 0.5f + x * 0.1f) * 0.2f;

    // Draw building with neon outlines
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLineWidth(3.0f);  // Thicker lines for better visibility

    // Building outline color - hot pink (classic retrowave color)
    RetroColor::Pink(time, 0.95f);

    // Front face - vertical edges
    glBegin(GL_LINES);
    // Left vertical edge
    glVertex3f(x - halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x - halfWidth, height + buildingOffset, z + halfDepth);

    // Right vertical edge
    glVertex3f(x + halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x + halfWidth, height + buildingOffset, z + halfDepth);

    // Horizontal edges - front face
    glVertex3f(x - halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z + halfDepth);

    glVertex3f(x - halfWidth, height + buildingOffset, z + halfDepth);
    glVertex3f(x + halfWidth, height + buildingOffset, z + halfDepth);

    // Back face - vertical edges
    glVertex3f(x - halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x - halfWidth, height + buildingOffset, z - halfDepth);

    glVertex3f(x + halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, height + buildingOffset, z - halfDepth);

    // Horizontal edges - back face
    glVertex3f(x - halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z - halfDepth);

    glVertex3f(x - halfWidth, height + buildingOffset, z - halfDepth);
    glVertex3f(x + halfWidth, height + buildingOffset, z - halfDepth);

    // Connect front to back - top edges
    glVertex3f(x - halfWidth, height + buildingOffset, z + halfDepth);
    glVertex3f(x - halfWidth, height + buildingOffset, z - halfDepth);

    glVertex3f(x + halfWidth, height + buildingOffset, z + halfDepth);
    glVertex3f(x + halfWidth, height + buildingOffset, z - halfDepth);
    glEnd();

    // Add outline glow for buildings
    glLineWidth(5.0f);
    RetroColor::Pink(time, 0.25f);  // Pink glow

    // Redraw front edges with glow
    glBegin(GL_LINES);
    // Left vertical edge
    glVertex3f(x - halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x - halfWidth, height + buildingOffset, z + halfDepth);

    // Right vertical edge
    glVertex3f(x + halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x + halfWidth, height + buildingOffset, z + halfDepth);

    // Top horizontal edge
    glVertex3f(x - halfWidth, height + buildingOffset, z + halfDepth);
    glVertex3f(x + halfWidth, height + buildingOffset, z + halfDepth);
    glEnd();
    glLineWidth(3.0f);

    // Draw windows - improved symmetrical version
    // Calculate perfect grid for windows
    int numFloors = static_cast<int>(height / 2.5f);
    int windowsPerFloor = static_cast<int>(width / 1.2f);

    // Ensure minimum number of windows
    numFloors = std::max(numFloors, 3);
    windowsPerFloor = std::max(windowsPerFloor, 2);

    // Window properties
    float windowWidth = width / (windowsPerFloor + 1);
    float windowHeight = windowWidth * 1.5f; // Rectangular windows
    float floorHeight = (height - 2.0f) / numFloors;

    float windowPulse = 0.7f + 0.3f * sinf(time * 1.5f);
    float globalWindowIntensity = 0.6f + 0.4f * sinf(time * 0.3f); // Stronger building pulse

    // Set window colors - use only classic retrowave colors
    const int numColors = 3;
    float colors[numColors][3] = {
        {0.0f, 0.9f, 1.0f},  // Brighter Cyan/Teal
        {1.0f, 0.8f, 0.0f},  // Golden/Orange
        {1.0f, 0.3f, 0.7f}   // Hot Pink/Magenta
    };

    // Determine color palette for this building based on its position
    int buildingColorScheme = static_cast<int>(fabs(x * 1000)) % numColors;

    // Draw windows on front face only, in perfect grid
    for (int floor = 0; floor < numFloors; floor++) {
        float floorY = 2.0f + floor * floorHeight;

        for (int w = 0; w < windowsPerFloor; w++) {
            // Calculate window position
            float windowX = x - halfWidth + (width / (windowsPerFloor + 1)) * (w + 1);
            float windowY = floorY + floorHeight * 0.5f;

            // Skip some windows randomly but consistently for this building (based on position)
            int hash = static_cast<int>((windowX * 100 + windowY * 50 + x * z * 10) * 10) % 10;
            if (hash < 3 && floor > 0) continue; // 30% chance of missing window except on first floor

            // Draw window outline (black)
            glColor4f(0.0f, 0.0f, 0.0f, 0.9f);
            glBegin(GL_QUADS);
            glVertex3f(windowX - windowWidth/2, windowY - windowHeight/2, z + halfDepth + 0.01f);
            glVertex3f(windowX + windowWidth/2, windowY - windowHeight/2, z + halfDepth + 0.01f);
            glVertex3f(windowX + windowWidth/2, windowY + windowHeight/2, z + halfDepth + 0.01f);
            glVertex3f(windowX - windowWidth/2, windowY + windowHeight/2, z + halfDepth + 0.01f);
            glEnd();

            // Determine window color - vary by floor for a pattern
            int colorIndex = (buildingColorScheme + floor) % numColors;

            // Create lighting pattern across the building (brightest in middle)
            float centerFactor = 1.0f - 2.0f * fabs((w + 0.5f) / windowsPerFloor - 0.5f); // 0-1-0 across building
            float heightFactor = 1.0f - (float)floor / numFloors * 0.3f; // brighter at bottom

            // Window light intensity
            float blink = 1.0f;
            if (hash == 8) { // 10% chance of blinking window
                blink = (sin(time * (5.0f + hash)) > 0) ? 1.0f : 0.3f;
            }

            float intensity = windowPulse * globalWindowIntensity * centerFactor * heightFactor * blink;

            // Set window color and draw
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glColor4f(
                colors[colorIndex][0] * intensity,
                colors[colorIndex][1] * intensity,
                colors[colorIndex][2] * intensity,
                0.95f
            );

            // Margin inside window
            float margin = windowWidth * 0.15f;

            // Draw window inner
            glBegin(GL_QUADS);
            glVertex3f(windowX - windowWidth/2 + margin, windowY - windowHeight/2 + margin, z + halfDepth + 0.02f);
            glVertex3f(windowX + windowWidth/2 - margin, windowY - windowHeight/2 + margin, z + halfDepth + 0.02f);
            glVertex3f(windowX + windowWidth/2 - margin, windowY + windowHeight/2 - margin, z + halfDepth + 0.02f);
            glVertex3f(windowX - windowWidth/2 + margin, windowY + windowHeight/2 - margin, z + halfDepth + 0.02f);
            glEnd();

            // Add window glow
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            float glowIntensity = intensity * 0.6f;

            // Stronger glow
            glColor4f(
                colors[colorIndex][0],
                colors[colorIndex][1],
                colors[colorIndex][2],
                glowIntensity
            );

            // Draw simple glow around window
            float glowSize = windowWidth * 2.0f;  // Larger glow
            glBegin(GL_QUADS);
            glVertex3f(windowX - glowSize/2, windowY - glowSize/2, z + halfDepth + 0.015f);
            glVertex3f(windowX + glowSize/2, windowY - glowSize/2, z + halfDepth + 0.015f);
            glVertex3f(windowX + glowSize/2, windowY + glowSize/2, z + halfDepth + 0.015f);
            glVertex3f(windowX - glowSize/2, windowY + glowSize/2, z + halfDepth + 0.015f);
            glEnd();
        }
    }

    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

void drawGrid(float size, int divisions) {
    float step = size / divisions;
    float halfSize = size / 2.0f;
    float startY = 0.0f;
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Enable blending for grid glow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Animation offset with smooth movement
    float offsetZ = fmodf(gridOffset * step, step);
    float speedFactor = 1.0f + 0.5f * sinf(time * 0.3f);
    offsetZ *= speedFactor;

    // Draw grid lines along Z axis (pink/magenta)
    for (int i = 0; i <= divisions; i++) {
        float x = -halfSize + i * step;
        float brightness = 1.0f - fabsf(x) / halfSize; // Fade out at edges

        // Skip some lines for a cleaner look
        if (i % 2 != 0 && abs(i - divisions/2) > 5) continue;

        // Adjust color for neon effect - more vibrant magenta
        float pulse = 0.7f + 0.3f * sinf(time * 2.0f + i * 0.1f);
        float alpha = 0.4f + 0.6f * brightness * pulse;
        RetroColor::Pink(time, alpha);

        // Draw thicker line
        glLineWidth(2.5f);
        glBegin(GL_LINES);
        glVertex3f(x, startY, -halfSize + offsetZ);
        glVertex3f(x, startY, halfSize + offsetZ);
        glEnd();
    }

    // Draw grid lines along X axis (cyan/blue)
    for (int i = 0; i <= divisions; i++) {
        float z = -halfSize + i * step + offsetZ;
        if (z > halfSize) continue;

        float brightness = 1.0f - fabsf(z) / halfSize; // Fade out at distance

        // Skip some lines for a cleaner look
        if (i % 2 != 0 && i > 5) continue;

        float pulse = 0.7f + 0.3f * sinf(time * 2.0f + i * 0.1f + 1.5f);
        float alpha = 0.4f + 0.6f * brightness * pulse;
        RetroColor::Cyan(time, alpha);

        // Draw thicker line
        glLineWidth(2.5f);
        glBegin(GL_LINES);
        glVertex3f(-halfSize, startY, z);
        glVertex3f(halfSize, startY, z);
        glEnd();
    }

    glDisable(GL_BLEND);
    glLineWidth(1.0f);
}

void drawSpinners() {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Draw each spinner
    for (size_t i = 0; i < spinners.size(); i++) {
        drawSpinner(spinners[i], time);
    }
}

void drawSpinner(const Spinner& spinner, float time) {
    glPushMatrix();
    glTranslatef(spinner.x, spinner.y, spinner.z);
    glRotatef(spinner.rotation, 0.0f, 0.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    float pulse = 0.7f + 0.3f * sinf(time * 2.0f);
    int segments = 24;
    float radius = spinner.radius;

    if (spinner.type == 0) {  // Circular spinner
        // Draw a circular spinner
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++) {
            float angle = static_cast<float>(i) / segments * 2.0f * M_PI;

            if (spinner.isPink) {
                RetroColor::Pink(time, 0.8f);
            } else {
                RetroColor::Cyan(time, 0.8f);
            }

            float x = radius * cosf(angle);
            float y = radius * sinf(angle);
            glVertex3f(x, y, 0.0f);
        }
        glEnd();

        // Draw spokes
        glBegin(GL_LINES);
        for (int i = 0; i < segments/4; i++) {
            float angle = static_cast<float>(i) / (segments/4) * 2.0f * M_PI;

            if (spinner.isPink) {
                RetroColor::Pink(time, 0.5f);
            } else {
                RetroColor::Cyan(time, 0.5f);
            }

            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(radius * cosf(angle), radius * sinf(angle), 0.0f);
        }
        glEnd();

    } else {  // Spiral spinner
        // Draw a spiral spinner
        int rings = 5;
        float ringStep = radius / rings;

        for (int r = 0; r < rings; r++) {
            float innerRadius = r * ringStep;
            float outerRadius = (r + 1) * ringStep;

            // Make the spiral
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i <= segments; i++) {
                float angle = static_cast<float>(i) / segments * 2.0f * M_PI;

                // Alternate colors along the spiral
                if ((r + i) % 2 == 0) {
                    RetroColor::Pink(time, 0.8f);
                } else {
                    RetroColor::Cyan(time, 0.8f);
                }

                float radius = innerRadius + (outerRadius - innerRadius) * i / segments;
                float x = radius * cosf(angle);
                float y = radius * sinf(angle);
                glVertex3f(x, y, 0.0f);
            }
            glEnd();
        }

        // Draw a circular outline
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++) {
            float angle = static_cast<float>(i) / segments * 2.0f * M_PI;

            if (i % 2 == 0) {
                RetroColor::Pink(time, 0.8f);
            } else {
                RetroColor::Cyan(time, 0.8f);
            }

            float x = radius * cosf(angle);
            float y = radius * sinf(angle);
            glVertex3f(x, y, 0.0f);
        }
        glEnd();
    }

    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glPopMatrix();
}

// Updated tunnel function to make it bigger
void drawTunnel(float radius, int segments, int rings) {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Position the tunnel in the sky - adjusted position for bigger tunnel
    glPushMatrix();
    glTranslatef(0.0f, 40.0f, -90.0f); // Moved higher and farther back

    // Rotate for better visibility
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f);

    // Spin the tunnel
    glRotatef(vortexAngle * 0.2f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // INCREASED RADIUS from 30.0f to 50.0f to make the tunnel bigger
    radius = 50.0f;

    // Draw tunnel grid lines
    glLineWidth(2.0f);

    // Draw radial lines
    for (int i = 0; i < segments; i++) {
        float angle = static_cast<float>(i) / segments * 2.0f * M_PI;
        float x = radius * cosf(angle);
        float y = radius * sinf(angle);

        // Pink for odd radials, blue for even
        if (i % 2 == 0) {
            RetroColor::Pink(time, 0.8f);
        } else {
            RetroColor::Cyan(time, 0.8f);
        }

        glBegin(GL_LINE_STRIP);
        for (int r = 0; r < rings; r++) {
            // Increased depth range for a deeper tunnel
            float depth = -50.0f + r * 3.0f + tunnelDepth;
            float scaleFactor = (1.0f - r / (float)rings) * 0.9f + 0.1f;

            // Create a perspective effect
            float currX = x * scaleFactor;
            float currY = y * scaleFactor;

            glVertex3f(currX, currY, depth);
        }
        glEnd();
    }

    // Draw concentric rings with increased count
    for (int r = 0; r < rings + 5; r++) { // Added 5 more rings
        // Increased depth range
        float depth = -50.0f + r * 3.0f + tunnelDepth;
        float scaleFactor = (1.0f - r / (float)rings) * 0.9f + 0.1f;

        // Alternate between pink and blue rings
        if (r % 2 == 0) {
            float pulse = 0.7f + 0.3f * sinf(time * 2.0f + r * 0.3f);
            RetroColor::Pink(time, 0.7f - (float)r/rings * 0.5f);
        } else {
            float pulse = 0.7f + 0.3f * sinf(time * 2.0f + r * 0.3f + M_PI);
            RetroColor::Cyan(time, 0.7f - (float)r/rings * 0.5f);
        }

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++) {
            float angle = static_cast<float>(i) / segments * 2.0f * M_PI;
            float currX = radius * scaleFactor * cosf(angle);
            float currY = radius * scaleFactor * sinf(angle);
            glVertex3f(currX, currY, depth);
        }
        glEnd();
    }

    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawCar(const Car& car) {
    float x = car.x;
    float z = car.z;
    float carLength = 4.0f;
    float carWidth = 2.0f;
    float carHeight = 1.2f;

    // Add bobbing animation
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float verticalOffset = sinf(time * 4.0f + x) * 0.1f;

    // Car color with pulse
    float pulseIntensity = 0.2f * sinf(time * 3.0f) + 0.8f;

    glPushMatrix();
    glTranslatef(x, 0.5f + verticalOffset, z);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Draw car outline (neon style)
    glLineWidth(2.5f);  // Thicker lines

    // Car outline color
    if (car.isBlue) {
        RetroColor::Cyan(time, 0.95f);
    } else {
        RetroColor::Gold(time, 0.95f);
    }

    // Bottom outline
    glBegin(GL_LINE_LOOP);
    glVertex3f(-carWidth/2, 0.0f, -carLength/2);
    glVertex3f(carWidth/2, 0.0f, -carLength/2);
    glVertex3f(carWidth/2, 0.0f, carLength/2);
    glVertex3f(-carWidth/2, 0.0f, carLength/2);
    glEnd();

    // Top outline
    glBegin(GL_LINE_LOOP);
    glVertex3f(-carWidth/2, carHeight, -carLength/2);
    glVertex3f(carWidth/2, carHeight, -carLength/2);
    glVertex3f(carWidth/2, carHeight, carLength/2 - 1.0f);
    glVertex3f(-carWidth/2, carHeight, carLength/2 - 1.0f);
    glEnd();

    // Connect bottom to top
    glBegin(GL_LINES);
    // Front-left
    glVertex3f(-carWidth/2, 0.0f, carLength/2);
    glVertex3f(-carWidth/2, carHeight, carLength/2 - 1.0f);

    // Front-right
    glVertex3f(carWidth/2, 0.0f, carLength/2);
    glVertex3f(carWidth/2, carHeight, carLength/2 - 1.0f);

    // Back-left
    glVertex3f(-carWidth/2, 0.0f, -carLength/2);
    glVertex3f(-carWidth/2, carHeight, -carLength/2);

    // Back-right
    glVertex3f(carWidth/2, 0.0f, -carLength/2);
    glVertex3f(carWidth/2, carHeight, -carLength/2);
    glEnd();

    // Add car glow
    glLineWidth(4.0f);
    if (car.isBlue) {
        RetroColor::Cyan(time, 0.3f);
    } else {
        RetroColor::Gold(time, 0.3f);
    }

    // Bottom outline glow
    glBegin(GL_LINE_LOOP);
    glVertex3f(-carWidth/2, 0.0f, -carLength/2);
    glVertex3f(carWidth/2, 0.0f, -carLength/2);
    glVertex3f(carWidth/2, 0.0f, carLength/2);
    glVertex3f(-carWidth/2, 0.0f, carLength/2);
    glEnd();
    glLineWidth(2.5f);

    // Draw headlights and taillights
    if (car.isBlue) {
        // Blue car with blue headlights
        glColor3f(0.0f, 0.9f, 1.0f);
    } else {
        // Orange car with yellow/orange headlights
        glColor3f(1.0f, 0.8f, 0.3f);
    }

    // Headlights
    glPointSize(5.0f);  // Bigger, brighter lights
    glBegin(GL_POINTS);
    glVertex3f(-carWidth/3, carHeight/3, carLength/2 + 0.1f);
    glVertex3f(carWidth/3, carHeight/3, carLength/2 + 0.1f);
    glEnd();

    // Add headlight glow
    if (car.isBlue) {
        glColor4f(0.0f, 0.9f, 1.0f, 0.5f);
    } else {
        glColor4f(1.0f, 0.8f, 0.3f, 0.5f);
    }
    glPointSize(10.0f);  // Big glow
    glBegin(GL_POINTS);
    glVertex3f(-carWidth/3, carHeight/3, carLength/2 + 0.1f);
    glVertex3f(carWidth/3, carHeight/3, carLength/2 + 0.1f);
    glEnd();

    // Taillights
    if (car.isBlue) {
        glColor3f(0.0f, 0.5f, 1.0f);
    } else {
        glColor3f(1.0f, 0.2f, 0.2f);
    }

    glPointSize(4.0f);
    glBegin(GL_POINTS);
    glVertex3f(-carWidth/3, carHeight/3, -carLength/2 - 0.1f);
    glVertex3f(carWidth/3, carHeight/3, -carLength/2 - 0.1f);
    glEnd();

    // Draw ground light trails - ENHANCED
    float trailIntensity = 0.8f + 0.2f * sinf(time * 5.0f);

    // Draw longer, more vibrant trails
    if (car.isBlue) {
        // Blue car with cyan trail
        glBegin(GL_QUADS);
        glColor4f(0.0f, 0.8f * trailIntensity, 1.0f * trailIntensity, 0.8f);
        glVertex3f(-carWidth/4, 0.05f, -carLength/2);
        glVertex3f(carWidth/4, 0.05f, -carLength/2);
        glColor4f(0.0f, 0.8f * trailIntensity * 0.3f, 1.0f * trailIntensity * 0.3f, 0.0f); // Fade to transparent
        glVertex3f(carWidth/4, 0.05f, -carLength/2 - 20.0f); // Longer trail
        glVertex3f(-carWidth/4, 0.05f, -carLength/2 - 20.0f);
        glEnd();
    } else {
        // Orange car with orange/red trail
        glBegin(GL_QUADS);
        glColor4f(1.0f * trailIntensity, 0.5f * trailIntensity, 0.0f, 0.8f);
        glVertex3f(-carWidth/4, 0.05f, -carLength/2);
        glVertex3f(carWidth/4, 0.05f, -carLength/2);
        glColor4f(1.0f * trailIntensity * 0.3f, 0.5f * trailIntensity * 0.3f, 0.0f, 0.0f); // Fade to transparent
        glVertex3f(carWidth/4, 0.05f, -carLength/2 - 20.0f); // Longer trail
        glVertex3f(-carWidth/4, 0.05f, -carLength/2 - 20.0f);
        glEnd();
    }

    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glPopMatrix();
}

void drawSky() {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Draw stars
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (size_t i = 0; i < stars.size(); i++) {
        const Star& star = stars[i];

        // Twinkling effect
        float twinkleSpeed = 3.0f + (i % 5) * 1.0f;
        float twinkle = 0.5f + 0.5f * sinf(time * twinkleSpeed + i * 0.1f);
        float brightness = star.brightness * twinkle;

        // Variable star size
        glPointSize(star.size * (0.8f + 0.4f * twinkle));

        // Star color
        if (star.colorType < 7) { // White/blue
            glColor3f(0.8f + 0.2f * twinkle, 0.8f + 0.2f * twinkle, 1.0f);
        } else if (star.colorType < 9) { // Yellow/orange
            glColor3f(1.0f, 0.7f + 0.3f * twinkle, 0.4f * twinkle);
        } else { // Red
            glColor3f(1.0f, 0.3f * twinkle, 0.2f * twinkle);
        }

        glBegin(GL_POINTS);
        glVertex3f(star.x, star.y, star.z);
        glEnd();

        // Add glow for bright stars
        if (star.brightness > 0.8f) {
            float glowSize = star.size * 3.0f * twinkle;

            if (star.colorType < 7) {
                glColor4f(0.6f, 0.6f, 1.0f, 0.2f * brightness);
            } else if (star.colorType < 9) {
                glColor4f(1.0f, 0.7f, 0.3f, 0.2f * brightness);
            } else {
                glColor4f(1.0f, 0.3f, 0.2f, 0.2f * brightness);
            }

            glPointSize(glowSize);
            glBegin(GL_POINTS);
            glVertex3f(star.x, star.y, star.z);
            glEnd();
        }
    }

    glDisable(GL_BLEND);
}

// New function to draw a pyramid shape
void drawPyramid(float time) {
    glPushMatrix();

    // Position the pyramid in the sky
    float hoverY = 20.0f + sinf(time * 0.5f) * 3.0f; // Hovering effect
    glTranslatef(-30.0f, hoverY, -40.0f);

    // Rotate the pyramid
    glRotatef(time * 20.0f, 0.0f, 1.0f, 0.2f);

    // Scale the pyramid
    float scale = 3.0f + sinf(time * 0.7f) * 0.5f; // Pulsating scale
    glScalef(scale, scale, scale);

    // Set material properties using retrowave color palette
    glEnable(GL_LIGHTING);

    // Use the retrowave colors - alternate between pink and cyan
    float pulse = 0.7f + 0.3f * sinf(time * 2.0f);

    // Choose color based on time for pulsing effect
    bool usePink = (sinf(time * 0.5f) > 0);
    GLfloat pyramidColor[4];

    if (usePink) {
        // Hot pink (classic retrowave color)
        RetroColor::getPinkMaterial(time, 1.0f, pyramidColor);
    } else {
        // Cyan (classic retrowave color)
        RetroColor::getCyanMaterial(time, 1.0f, pyramidColor);
    }

    GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat shininess[] = {50.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pyramidColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

    // Disable lighting temporarily for neon wireframe effect
    glDisable(GL_LIGHTING);

    // Draw pyramid wireframe - base is a square
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Draw wireframe with thicker lines for neon effect
    glLineWidth(2.5f);

    if (usePink) {
        RetroColor::Pink(time, 0.95f);
    } else {
        RetroColor::Cyan(time, 0.95f);
    }

    // Draw base
    glBegin(GL_LINE_LOOP);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glEnd();

    // Draw edges from base to apex
    glBegin(GL_LINES);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(0.0f, 2.0f, 0.0f);

    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(0.0f, 2.0f, 0.0f);

    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(0.0f, 2.0f, 0.0f);

    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(0.0f, 2.0f, 0.0f);
    glEnd();

    // Add glow effect
    glLineWidth(4.0f);

    if (usePink) {
        RetroColor::Pink(time, 0.3f); // Pink glow
    } else {
        RetroColor::Cyan(time, 0.3f); // Cyan glow
    }

    // Redraw lines with glow
    glBegin(GL_LINES);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(0.0f, 2.0f, 0.0f);

    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(0.0f, 2.0f, 0.0f);
    glEnd();

    // Reset line width
    glLineWidth(1.0f);

    // Re-enable lighting for solid model
    glEnable(GL_LIGHTING);

    // Draw semi-transparent faces for the pyramid
    pyramidColor[3] = 0.3f; // Make it semi-transparent
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pyramidColor);

    // Draw pyramid faces (triangles)
    glBegin(GL_TRIANGLES);
    // Front face
    glNormal3f(0.0f, 0.5f, 0.5f);  // Approximate normal
    glVertex3f(0.0f, 2.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    // Right face
    glNormal3f(0.5f, 0.5f, 0.0f);  // Approximate normal
    glVertex3f(0.0f, 2.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    // Back face
    glNormal3f(0.0f, 0.5f, -0.5f);  // Approximate normal
    glVertex3f(0.0f, 2.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);

    // Left face
    glNormal3f(-0.5f, 0.5f, 0.0f);  // Approximate normal
    glVertex3f(0.0f, 2.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glEnd();

    glDisable(GL_BLEND);
    glPopMatrix();
}

// Updated torus function to use consistent retro wave colors
void drawTorus(float time) {
    glPushMatrix();

    // Position the torus
    float orbitX = sinf(time * 0.4f) * 20.0f;
    float orbitZ = cosf(time * 0.4f) * 20.0f;
    glTranslatef(orbitX, 15.0f, -30.0f + orbitZ);

    // Rotate the torus continuously
    glRotatef(time * 50.0f, 1.0f, 0.5f, 0.0f);

    // Set material properties using retrowave colors
    glEnable(GL_LIGHTING);

    // Calculate color pulse
    float pulse = 0.7f + 0.3f * sinf(time * 1.5f);

    // Alternate between retrowave colors
    int colorChoice = static_cast<int>(time * 0.2f) % 3;
    GLfloat torusColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    switch (colorChoice) {
        case 0: // Hot magenta/pink
            RetroColor::getPinkMaterial(time, 1.0f, torusColor);
            break;
        case 1: // Cyan/blue
            RetroColor::getCyanMaterial(time, 1.0f, torusColor);
            break;
        case 2: // Vibrant yellow/gold
            RetroColor::getGoldMaterial(time, 1.0f, torusColor);
            break;
    }

    GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat shininess[] = {40.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, torusColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

    // Temporarily disable lighting for wireframe effect
    glDisable(GL_LIGHTING);

    // Draw wireframe with thicker lines for neon effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLineWidth(2.5f);

    // Use same color for wireframe as selected material
    switch (colorChoice) {
        case 0:
            RetroColor::Pink(time, 0.95f);
            break;
        case 1:
            RetroColor::Cyan(time, 0.95f);
            break;
                case 2:
            RetroColor::Gold(time, 0.95f);
            break;
    }

    // Draw wireframe torus using glutWireTorus with retrowave colors
    glutWireTorus(1.0f, 4.0f, 16, 48);

    // Add glow effect
    glLineWidth(4.0f);

    switch (colorChoice) {
        case 0:
            RetroColor::Pink(time, 0.3f);
            break;
        case 1:
            RetroColor::Cyan(time, 0.3f);
            break;
        case 2:
            RetroColor::Gold(time, 0.3f);
            break;
    }

    // Redraw some rings for glow effect
    glutWireTorus(1.1f, 4.1f, 8, 24);

    // Reset line width
    glLineWidth(1.0f);

    // Re-enable lighting for solid model
    glEnable(GL_LIGHTING);

    // Draw solid torus with transparency for glow effect
    torusColor[3] = 0.2f; // Make it semi-transparent
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, torusColor);

    glutSolidTorus(0.8f, 4.2f, 16, 48); // Different proportions for effect

    glDisable(GL_BLEND);
    glPopMatrix();
}

void calculateFPS() {
    frameCount++;
    currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float timeInterval = currentTime - previousTime;

    if (timeInterval >= 1.0f) {
        fps = frameCount / timeInterval;
        previousTime = currentTime;
        frameCount = 0;

        // Update window title with FPS
        char title[64];
        snprintf(title, sizeof(title), "Retro Wave city - 221003166 - 221001810 - FPS: %.1f", fps);
        glutSetWindowTitle(title);
    }
}
