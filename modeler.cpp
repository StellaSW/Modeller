/*
Simple 3d Modeler
Written by: Shengyu Wu(400075340)
            Zuodong Xie(1414623)
*/

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#endif

#include <iostream>
#include <ctime>
#include <cmath>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

/*globals*/
const int map_size = 100;
int width = 800;
int height = 600;

//for object selection and creation
int obj_index = 0;
int selected = 0;
int target = 0;
int deleted = 0;
int nextMaterial = 0;
bool hit = false;
float bound = 1;

//mouse control
int mouseX = 0;
int mouseY = 0;

//0-type, 1,2,3-translate, 4,5,6-rotation, 7-scale, 8-material, 9-volume
float Object[100][9];

float eyeRotate[] = {0, 0, 0};
float eye[] = {50, 10, 90};
float sceneSpeed = 1;

float switchlight = 0;

//0-black rubber 1-cyan rubber 2-green rubber 3-red rubber 4-white rubber
float Material[5][10] = {{0.02, 0.02, 0.02, 0.01, 0.01, 0.01, 0.4, 0.4, 0.4, 0.078},
                         {0, 0.05, 0.05, 0.4, 0.5, 0.5, 0.04, 0.7, 0.7, 0.078},
                         {0, 0.05, 0, 0.4, 0.5, 0.4, 0.04, 0.7, 0.04, 0.078},
                         {0.05, 0, 0, 0.5, 0.4, 0.4, 0.7, 0.04, 0.04, 0.078},
                         {0.05, 0.05, 0.05, 0.5, 0.5, 0.5, 0.7, 0.7, 0.7, 0.078}};

float light_pos[] = {0, 2, 1, 0};
float amb0[4] = {0.1, 0.1, 0.2, 1};
float diff0[4] = {1, 0, 0, 1};
float spec0[4] = {0, 0, 0, 1};

float light_pos1[] = {0, 2, 1, 0};
float amb1[4] = {0, 0.1, 0.1, 1};
float diff1[4] = {0, 1, 0, 1};
float spec1[4] = {0, 0, 0, 1};

//file pointer
FILE *fptr;

void WriteFile(void)
{
    //open file and write, update
    fptr = fopen("file", "w+");
    char index[20] = {0};
    sprintf(index, "%i", obj_index);
    fputs(index, fptr);
    fputs("\n", fptr);
    char str[20] = {0};
    sprintf(str, "%i", selected);
    fputs(str, fptr);
    fputs("\n", fptr);

    for (int i = 0; i < obj_index; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            char obj[20] = {0};
            sprintf(obj, "%f", Object[i][j]);
            fputs(obj, fptr);
            fputs("\n", fptr);
        }
    }
    fclose(fptr);
}

void ReadFile(void)
{
    //open file and read
    fptr = fopen("file", "r");
    char index[20] = {0};
    fgets(index, 255, (FILE *)fptr);
    //transfer to integer
    obj_index = atoi(index);
    char select[20] = {0};
    fgets(select, 255, (FILE *)fptr);
    //transfer to integer
    selected = atoi(select);
    for (int i = 0; i < obj_index; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            char Obj[20] = {0};
            fgets(Obj, 200, (FILE *)fptr);
            //convert string to double
            Object[i][j] = atof(Obj);
        }
    }
    fclose(fptr);
}

void initialize(void)
{
    for (int i = 0; i < 10; i++)
    {
        Object[i][0] = -1;

        Object[i][7] = 0;

        Object[i][8] = -1;
    }
}

void Raypicking()
{
    //construct Ray
    double m_start[3], m_end[3], m_ray[3];
    double matModelView[16], matProjection[16];
    int viewPort[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, matModelView);
    glGetDoublev(GL_PROJECTION_MATRIX, matProjection);
    glGetIntegerv(GL_VIEWPORT, viewPort);

    //calculate near point
    gluUnProject(mouseX, mouseY, 0.0, matModelView, matProjection, viewPort, &m_start[0], &m_start[1], &m_start[2]);
    //calculate far point
    gluUnProject(mouseX, mouseY, 1.0, matModelView, matProjection, viewPort, &m_end[0], &m_end[1], &m_end[2]);

    //create ray from m_start to m_end
    m_ray[0] = m_end[0] - m_start[0];
    m_ray[1] = m_end[1] - m_start[1];
    m_ray[2] = m_end[2] - m_start[2];

    //turn ray into unit ray
    double m = sqrt(m_ray[0] * m_ray[0] + m_ray[1] * m_ray[1] + m_ray[2] * m_ray[2]);
    m_ray[0] /= m;
    m_ray[1] /= m;
    m_ray[2] /= m;

    //assume the object with a box of radius 1 centered around the object centered
    //calculate intersection
    for (int i = 0; i < 100; i++)
    {
        //calculate t value from z direction;
        double t = ((Object[i][3]) - m_start[2]) / m_ray[2];

        //use t value to find x and y of our intersection point
        double m_x = m_start[0] + t * m_ray[0];
        double m_y = m_start[1] + t * m_ray[1];
        double m_z = m_start[2] + t * m_ray[2];

        //location, if point is inside box centered at object
        if (Object[i][1] - bound < m_x < Object[i][1] + bound &&
            Object[i][2] - bound < m_y < Object[i][2] + bound &&
            Object[i][3] - bound < m_z < Object[i][3] + bound)
        {
            target = i;
            hit = true;
            bound = 1 + Object[i][7];
        }
        else
            hit = false;
    }
}

//draw bounding wireframe box
void drawBox(int type)
{
    glPushMatrix();
    //xyz for translation
    float x = Object[type][1];
    float y = Object[type][2];
    float z = Object[type][3];
    glColor3f(0, 1, 0);
    glTranslatef(x, y, z);
    //matrix for rotation
    glRotatef(Object[type][4], 1, 0, 0);
    glRotatef(Object[type][5], 0, 1, 0);
    glRotatef(Object[type][6], 0, 0, 1);
    float s = 1 + Object[type][7];
    glBegin(GL_LINES);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, s, -s);
    glVertex3f(-s, -s, -s);
    glVertex3f(s, -s, -s);
    glVertex3f(-s, -s, -s);
    //current point
    glVertex3f(-s, -s, s);
    glVertex3f(s, s, s);
    //current point
    glVertex3f(s, s, -s);
    glVertex3f(s, s, s);
    //current point
    glVertex3f(s, -s, s);
    glVertex3f(s, s, s);
    //current point
    glVertex3f(-s, s, s);
    glVertex3f(-s, -s, s);
    //current point
    glVertex3f(-s, s, s);
    glVertex3f(-s, -s, s);
    //current point
    glVertex3f(s, -s, s);
    glVertex3f(-s, s, s);
    //current point
    glVertex3f(-s, s, -s);
    glVertex3f(-s, s, -s);
    //current point
    glVertex3f(s, s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, -s, -s);
    glVertex3f(s, -s, -s);
    //current point
    glVertex3f(s, -s, s);
    glEnd();
    glPopMatrix();
}

//add new object and select
void add(int type)
{
    //add selected object
    selected = obj_index;
    Object[obj_index][0] = type;
    Object[obj_index][1] = map_size / 2;
    Object[obj_index][2] = 1;
    Object[obj_index][3] = map_size / 2;
    obj_index++;
}

void deleteSelected(void)
{
    for (int i = deleted; i < obj_index; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            Object[i][j] = Object[i + 1][j];
        }
    }
    //delete selected object
    for (int j = 0; j < 9; j++)
    {
        Object[obj_index][j] = 0;
    }
    if (deleted == selected)
    {
        selected -= 1;
    }

    obj_index -= 1;
}

void setMaterial(int m)
{
    float amb[4] = {Material[m][0], Material[m][1], Material[m][2], 1.0f};
    float diff[4] = {Material[m][3], Material[m][4], Material[m][5], 1.0f};
    float spec[4] = {Material[m][6], Material[m][7], Material[m][8], 1.0f};
    float shine[1] = {Material[m][9]};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
}

void SetnextMaterial(int n)
{
    if ((obj_index) != (sizeof(Object) / 4 / 9))
    {
        Object[obj_index][8] = n;
    }
}

//Additional Custom Shape
void drawSnowMan()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    // Body
    glTranslatef(0.0f, 0.75f, 0.0f);
    glutSolidSphere(0.75f, 20, 20);

    // Head
    glTranslatef(0.0f, 1.0f, 0.0f);
    glutSolidSphere(0.25f, 20, 20);

    // Eyes
    glPushMatrix();
    glColor3f(0.0f, 0.0f, 0.0f);
    glTranslatef(0.05f, 0.10f, 0.18f);
    glutSolidSphere(0.05f, 10, 10);
    glTranslatef(-0.1f, 0.0f, 0.0f);
    glutSolidSphere(0.05f, 10, 10);
    glPopMatrix();

    // Nose
    glColor3f(1.0f, 0.5f, 0.5f);
    glutSolidCone(0.08f, 0.5f, 10, 2);
}

void drawObject(void)
{
    for (int i = 0; i < obj_index; i++)
    {
        glDisable(GL_COLOR_MATERIAL);
        glPushMatrix();
        float obj_x = Object[i][1];
        float obj_y = Object[i][2];
        float obj_z = Object[i][3];
        glTranslatef(obj_x, obj_y, obj_z);
        glRotatef(Object[i][4], 1, 0, 0);
        glRotatef(Object[i][5], 0, 1, 0);
        glRotatef(Object[i][6], 0, 0, 1);
        float obj_s = 1 + Object[i][7];
        glScalef(obj_s, obj_s, obj_s);

        glColor3f(0, 1, 1);

        if (Object[i][8] != -1)
        {
            setMaterial(Object[i][8]);
        }
        if (Object[i][0] == 0)
            glutSolidTeapot(1);
        if (Object[i][0] == 1)
            glutSolidCube(1);
        if (Object[i][0] == 2)
            glutSolidTorus(1, 1, 100, 100);
        if (Object[i][0] == 3)
            glutSolidCone(1, 1, 100, 100);
        if (Object[i][0] == 4)
            glutSolidSphere(1, 100, 100);
        if (Object[i][0] == 5)
            drawSnowMan();
        glPopMatrix();
    }
}

void drawMap()
{
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(-100.0f, 0.0f, -100.0f);
    glVertex3f(-100.0f, 0.0f, 100.0f);
    glVertex3f(100.0f, 0.0f, 100.0f);
    glVertex3f(100.0f, 0.0f, -100.0f);
    glEnd();
}

void init(void)
{
    glClearColor(0, 0, 0, 0);
    glColor3f(1, 1, 1);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45, 1, 1, 500);
}

//clean all
void reset(void)
{
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            Object[i][j] = 0;
        }
    }
    obj_index = 0;
    selected = 0;
    switchlight = 0;
    hit = false;
    initialize();
    eyeRotate[1] = 0;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'Q':
    case 'q':
    case 27:
        exit(0);
        break;
    case 'y':
    case 'Y':
        nextMaterial = 1;
    //add material
    case '1':
        if (nextMaterial == 1)
            SetnextMaterial(0);
        else
            Object[selected][8] = 0;
        nextMaterial = 0;
        break;
    case '2':
        if (nextMaterial == 1)
            SetnextMaterial(1);
        else
            Object[selected][8] = 1;
        nextMaterial = 0;
        break;
    case '3':
        if (nextMaterial == 1)
            SetnextMaterial(2);
        else
            Object[selected][8] = 2;
        nextMaterial = 0;
        break;
    case '4':
        if (nextMaterial == 1)
            SetnextMaterial(3);
        else
            Object[selected][8] = 3;
        nextMaterial = 0;
        break;
    case '5':
        if (nextMaterial == 1)
            SetnextMaterial(4);
        else
            Object[selected][8] = 4;
        nextMaterial = 0;
        break;
    //add objects
    case '6':
        add(0);
        break;
    case '7':
        add(1);
        break;
    case '8':
        add(2);
        break;
    case '9':
        add(3);
        break;
    case '0':
        add(4);
        break;
    case '-':
        add(5);
        break;
    //move around the selected object
    case 'w':
    case 'W':
        Object[selected][1] += 1;
        break;
    case 'z':
    case 'Z':
        Object[selected][1] -= 1;
    case 'd':
    case 'D':
        Object[selected][2] += 1;
        break;
    case 'a':
    case 'A':
        Object[selected][2] -= 1;
        break;
    case 'e':
    case 'E':
        Object[selected][3] += 1;
        break;
    case 'x':
    case 'X':
        Object[selected][3] -= 1;
        break;
    //scale up and down
    case 'o':
    case 'O':
        Object[selected][7] += 0.5;
        break;
    case 'p':
    case 'P':
        Object[selected][7] -= 0.5;
        break;
    //rotation
    case 'u':
    case 'U':
        Object[selected][4] += 1;
        break;
    case 'i':
    case 'I':
        Object[selected][4] -= 1;
        break;
    case 'j':
    case 'J':
        Object[selected][5] += 1;
        break;
    case 'k':
    case 'K':
        Object[selected][5] -= 1;
        break;
    case 'n':
    case 'N':
        Object[selected][6] += 1;
        break;
    case 'm':
    case 'M':
        Object[selected][6] -= 1;
        break;
    //switch between two lights
    case 't':
    case 'T':
        if (switchlight == 0)
        {
            switchlight = 1;
        }
        else if (switchlight == 1)
        {
            switchlight = 0;
        }
        break;
    //light control
    case 'g':
    case 'G':
        if (switchlight == 0)
            light_pos[0] -= 1;
        if (switchlight == 1)
            light_pos1[0] -= 1;
        break;
    case 'h':
    case 'H':
        if (switchlight == 0)
            light_pos[0] += 1;
        if (switchlight == 1)
            light_pos1[0] += 1;
        break;
    case 'v':
    case 'V':
        if (switchlight == 0)
            light_pos[1] -= 1;
        if (switchlight == 1)
            light_pos1[1] -= 1;
        break;
    case 'b':
    case 'B':
        if (switchlight == 0)
            light_pos[1] += 1;
        if (switchlight == 1)
            light_pos1[1] += 1;
        break;
    case 'r':
    case 'R':
        reset();
        break;
    case 's':
    case 'S':
        WriteFile();
        break;
    case 'l':
    case 'L':
        ReadFile();
        break;
    }
    glutPostRedisplay();
}

void special(int key, int x, int y)
{
    switch (key)
    {
    //control scene rotation
    case GLUT_KEY_UP:
        eyeRotate[0] += sceneSpeed;
        break;

    case GLUT_KEY_DOWN:
        eyeRotate[0] -= sceneSpeed;
        break;

    case GLUT_KEY_LEFT:
        eyeRotate[1] += sceneSpeed;
        break;

    case GLUT_KEY_RIGHT:
        eyeRotate[1] -= sceneSpeed;
        break;

    case GLUT_KEY_HOME:
        eyeRotate[2] += sceneSpeed;
        break;

    case GLUT_KEY_END:
        eyeRotate[2] -= sceneSpeed;
        break;
    }
    glutPostRedisplay();
}

void mouse(int btn, int state, int x, int y)
{
    mouseX = x;
    mouseY = height - y;
    switch (btn)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_UP)
        {
            selected = target;
            drawBox(target);
            Raypicking();
        }
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
        {
            deleted = target;
            deleteSelected();
            Raypicking();
        }
        break;
    }
}

void printText(void)
{
    glColor3f(0, 0, 0);

    //bitmap text
    glRasterPos2i(10, 110);
    std::string instrucString = "w,e,a,d,z,x: Translation; u,i,j,k,n,m : Rotation; o,p: Scale Up/Down ";
    for (std::string::iterator i = instrucString.begin(); i != instrucString.end(); ++i)
    {
        char c = *i;
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    glRasterPos2i(10, 90);
    std::string instrucString2 = "arrow key,home,end : Camera Control; g,h,v,b : Light Control; t : Switch Light";
    for (std::string::iterator i = instrucString2.begin(); i != instrucString2.end(); ++i)
    {
        char c = *i;
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    glRasterPos2i(10, 70);
    std::string instrucString3 = "1 ~ 5 : Add material; 6 ~ 0,- : Add object; r : Reset; s,l : Read or Write file";
    for (std::string::iterator i = instrucString3.begin(); i != instrucString3.end(); ++i)
    {
        char c = *i;
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    glRasterPos2i(10, 40);
    std::string instrucString4 = "Introduction:";
    for (std::string::iterator i = instrucString4.begin(); i != instrucString4.end(); ++i)
    {
        char c = *i;
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }
}

void display(void)
{
    glClearColor(0.6, 0.6, 0.6, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //enable two lights
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    glLightfv(GL_LIGHT0, GL_AMBIENT, amb0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glLightfv(GL_LIGHT1, GL_AMBIENT, amb1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diff1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, spec1);
    glLightfv(GL_LIGHT1, GL_POSITION, light_pos1);

    //backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(eye[0], eye[1], eye[2], map_size / 2, 0, map_size / 2, 0, 1, 0);

    glPushMatrix();

        glTranslatef(map_size / 2, 0, map_size / 2);
        glRotatef(eyeRotate[0], 1, 0, 0);
        glTranslatef(-map_size / 2, 0, -map_size / 2);

        glTranslatef(map_size / 2, 0, map_size / 2);
        glRotatef(eyeRotate[1], 0, 1, 0);
        glTranslatef(-map_size / 2, 0, -map_size / 2);

        glTranslatef(map_size / 2, 0, map_size / 2);
        glRotatef(eyeRotate[2], 0, 0, 1);
        glTranslatef(-map_size / 2, 0, -map_size / 2);

        drawMap();
        drawBox(selected);
        drawObject();

        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);

        glPushMatrix();

            glLoadIdentity();
            glOrtho(0.0, width, height, 0.0, -1.0, 10.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glDisable(GL_CULL_FACE);
            glClear(GL_DEPTH_BUFFER_BIT);
            printText();
            glMatrixMode(GL_PROJECTION);

        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_LIGHTING);

    glPopMatrix();
    glutSwapBuffers();
}

void FPS(int value)
{
    glutPostRedisplay();
    glutTimerFunc(17, FPS, 0); // 1sec = 1000, 60fptrs = 1000/60 = ~17
}

//callbacks registry
void callBackInit()
{
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutTimerFunc(0, FPS, 0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    //initialize window
    glutInitWindowSize(width, height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Simple Modeller");

    callBackInit();
    init();
    glEnable(GL_DEPTH_TEST);
    //starts the event loop
    glutMainLoop();

    return (0);
}