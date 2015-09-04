#include "stdafx.h"
#include "Managers.h"
#include "ElevationMap.h"

#define WINDOW_X 64
#define WINDOW_Y 64
#define WINDOW_W 800
#define WINDOW_H 600

Managers::ShaderManager* shaderManager;
ElevationMap* elevationMap;

void makeConsole() {
    FILE* pFile;
    AllocConsole();
    freopen_s(&pFile, "conin$", "r", stdin);
    freopen_s(&pFile, "conout$", "w", stdout);
    freopen_s(&pFile, "conout$", "w", stderr);
    printf("Debugging Window:\n");
}

void renderScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.8f, 0.3f, 0.4f, 1.0f);

    elevationMap->draw();

    glutSwapBuffers();
}

bool _tracked;
int _x;
int _y;
float _step = 1.0f;

void motionFunc(int x, int y)
{
    int dy = y - _y;
    _y = y;

    if (dy == 0)
        return;

    auto clamp = [](float a, float b, float c)
    {
        if (b < a) return a;
        else if (b > c) return c;
        else return b;
    };

    if (elevationMap->_camType == ElevationMap::PERSPECTIVE)
    {
        float fovy = elevationMap->_fovy;
        float newFovy = clamp(M_PI / 64, fovy + (float)dy / 64, M_PI * 63 / 64);
        elevationMap->_fovy = newFovy;
    }
    else if (elevationMap->_camType == ElevationMap::ORTHOGRAPHIC)
    {
        float m = pow(1.01f, dy);
        elevationMap->_left *= m;
        elevationMap->_right *= m;
        elevationMap->_bottom *= m;
        elevationMap->_top *= m;
    }
    glutPostRedisplay();
}

void passiveMotionFunc(int x, int y)
{
    if (_tracked)
    {
        int dx = x - _x;
        int dy = y - _y;

        elevationMap->panCamera(-(float)dy / 256, -(float)dx / 128);
    }

    _x = x;
    _y = y;

    if (x < WINDOW_W / 4 || x > WINDOW_W * 3 / 4)
    {
        glutWarpPointer(WINDOW_W / 2, _y);
        _x = WINDOW_W / 2;
    }
    if (y < WINDOW_H / 4 || y > WINDOW_H * 3 / 4)
    {
        glutWarpPointer(_x, WINDOW_H / 2);
        _y = WINDOW_H / 2;
    }

    _tracked = true;

    glutPostRedisplay();
}

void mouseFunc(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        switch (button)
        {
        case GLUT_RIGHT_BUTTON:

            elevationMap->toggleCameraType();
            glutMotionFunc(nullptr);
            break;

        case GLUT_LEFT_BUTTON:

            glutMotionFunc(motionFunc);
            
        }
    }
    glutPostRedisplay();
}

char _keys[3];
void keyboardFunc(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'r':
        elevationMap->moveCamera(glm::vec3(0, 0, _step));
        break;
    case 'd':
        elevationMap->moveCamera(glm::vec3(-_step, 0, 0));
        break;
    case 'f':
        elevationMap->moveCamera(glm::vec3(0, 0, -_step));
        break;
    case 'g':
        elevationMap->moveCamera(glm::vec3(_step, 0, 0));
        break;
    case ' ':
        elevationMap->shiftCamera(glm::vec3(0, _step, 0));
        break;
    case 'z':
        elevationMap->shiftCamera(glm::vec3(0, -_step, 0));
        break;
    }
    glutPostRedisplay();
}


int main(int argc, char **argv)
{
    makeConsole();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(WINDOW_X, WINDOW_Y);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutCreateWindow("Elevation Map Viewer");
    glutSetCursor(GLUT_CURSOR_NONE);

    glewInit();
    if (glewIsSupported("GL_VERSION_4_0"))
        std::cout << " GLEW Version is 4.0\n ";
    else
        std::cout << "GLEW 4.0 not supported\n ";

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //////////////////////////////////////////////////

    shaderManager = new Managers::ShaderManager;
    shaderManager->CreateProgram("shader", "Shaders/VertexShader.glsl", "Shaders/FragmentShader.glsl");

    elevationMap = new ElevationMap(64, 64, 1.0f, 1.0f);
    elevationMap->_aspect = (float)WINDOW_W / WINDOW_H;
    elevationMap->_shader = Managers::ShaderManager::GetShader("shader");
    elevationMap->randomize(0.0f, 8.0f);

    glutMouseFunc(mouseFunc);
    glutPassiveMotionFunc(passiveMotionFunc);
    glutKeyboardFunc(keyboardFunc);

    // register callbacks to glutMouseFunc(), etc...
    glutDisplayFunc(renderScene);

    glutMainLoop();

    return 0;
}