#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "inimigos.h"

#define MAX_VERTICES 10000
#define MAX_FACES 10000

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

enum Coords {X, Y, Z};

typedef struct vertice {
    float x, y, z;
} Vertice;

typedef struct textura {
    float u, v;
} Textura;

typedef struct normal {
    float x, y, z;
} Normal;

typedef struct face {
    int v[4];
    int t[4];
    int n[4];
} Face;

typedef struct player {
    GLfloat lookAt[3];
    GLfloat position[3];
    GLfloat sensibility;
    GLfloat speed;
} Player;

typedef struct mousePos {
    int x;
    int y;
} Mouse;

Vertice vertices[MAX_VERTICES];
Textura texturas[MAX_VERTICES];
Normal normais[MAX_VERTICES];
Face faces[MAX_FACES];
int qtdVertices = 0, qtdTexturas = 0, qtdNormais = 0, qtdFaces = 0;

Player p;
Mouse m;
int showCursor = 0;
GLfloat rotationY = -90.0f;
GLfloat rotationZ = -5.0f;

const int fireRating = 25;
int actualFrame = 0;
int shotAnim = 0;

Enemy* enemies;

void initEnemies() {
    int qtdDeInimigos = 3 + rand() % 3;

    for (int i = 0; i < qtdDeInimigos; i++) {
        float minX = -18.0f;
        float maxX = 18.0f;

        float minY = -8.0f;
        float maxY = 18.0f;

        float minZ = -20.0f;
        float maxZ = -16.0f;

        float x = minX + ((float)rand() / RAND_MAX) * (maxX - minX);
        float y = minY + ((float)rand() / RAND_MAX) * (maxY - minY);
        float z = minZ + ((float)rand() / RAND_MAX) * (maxZ - minZ);
        float colorR = (float)rand() / RAND_MAX;
        float colorG = (float)rand() / RAND_MAX;
        float colorB = (float)rand() / RAND_MAX;
        enemy e = {{x, y, z},
                    {colorR, colorG, colorB},
                    0.1f};

        enemies = addEnemy(enemies, e);
    }
}

void loadObject(const char *nomeArquivo) {
    FILE *fp = fopen(nomeArquivo, "r");
    if (!fp) {
        printf("Erro ao abrir o arquivo %s\n", nomeArquivo);
        exit(1);
    }

    char linha[128];
    while (fgets(linha, sizeof(linha), fp)) {
        if (strncmp(linha, "v ", 2) == 0) {
            Vertice vertice;
            sscanf(linha, "v %f %f %f", &vertice.x, &vertice.y, &vertice.z);
            vertices[qtdVertices++] = vertice;
        } else if (strncmp(linha, "vt ", 3) == 0) {
            Textura texCoord;
            sscanf(linha, "vt %f %f", &texCoord.u, &texCoord.v);
            texturas[qtdTexturas++] = texCoord;
        } else if (strncmp(linha, "vn ", 3) == 0) {
            Normal normal;
            sscanf(linha, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
            normais[qtdNormais++] = normal;
        } else if (strncmp(linha, "f ", 2) == 0) {
            Face face;
            int n = sscanf(linha, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                           &face.v[0], &face.t[0], &face.n[0],
                           &face.v[1], &face.t[1], &face.n[1],
                           &face.v[2], &face.t[2], &face.n[2],
                           &face.v[3], &face.t[3], &face.n[3]);
            // Ajuste para índices começando de zero
            for (int i = 0; i < 4; i++) {
                face.v[i] = (face.v[i] > 0) ? face.v[i] - 1 : -1;
                face.t[i] = (face.t[i] > 0) ? face.t[i] - 1 : -1;
                face.n[i] = (face.n[i] > 0) ? face.n[i] - 1 : -1;
            }
            faces[qtdFaces++] = face;
        }
    }

    fclose(fp);
}

void drawObject() {
    glPushMatrix();

    glTranslatef(p.position[X], p.position[Y] - 5, p.position[Z] - 15);

    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

    if(shotAnim) {
        glRotatef(rotationZ - (((float)fireRating - (float) actualFrame) / 2), 0.0f, 0.0f, 1.0f);
    } else {
        glRotatef(rotationZ, 0.0f, 0.0f, 1.0f);
    }

    for (int i = 0; i < qtdFaces; i++) {
        Face face = faces[i];
        glBegin(face.v[3] == -1 ? GL_TRIANGLES : GL_QUADS);
        for (int j = 0; j < 4 && face.v[j] != -1; j++) {
            if (face.n[j] != -1) {
                Normal normal = normais[face.n[j]];
                glNormal3f(normal.x, normal.y, normal.z);
            }
            if (face.t[j] != -1) {
                Textura texCoord = texturas[face.t[j]];
                glTexCoord2f(texCoord.u, texCoord.v);
            }
            Vertice vertice = vertices[face.v[j]];
            glVertex3f(vertice.x, vertice.y, vertice.z);
        }
        glEnd();
    }

    glPopMatrix();
    glLoadIdentity();
}

void initPlayer() {
    p.lookAt[X] = 0.0f;
    p.lookAt[Y] = 0.0f;
    p.lookAt[Z] = 0.0f;

    p.position[X] = 0.0f;
    p.position[Y] = 5.0f;
    p.position[Z] = 50.0f;

    p.sensibility = 0.5f;
    p.speed = 1.0f;

    m.x = -1;
    m.y = -1;
}

void renderText(GLfloat x, GLfloat y, char* text) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0.0, 0.0, 0.0);
    glRasterPos2f(x, y);

    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void initLighting() {
    // Luz ambiente global (suave e neutra)
    GLfloat globalAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // Configuração da GL_LIGHT0
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lightPosition[] = { 0.0f, 10.0f, 10.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    GLfloat pointLightDiffuse[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat pointLightSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat pointLightPosition[] = { 0.0f, 19.0f, 25.0f, 1.0f };

    glLightfv(GL_LIGHT1, GL_AMBIENT, pointLightDiffuse);  // Propriedade ambiente
    glLightfv(GL_LIGHT1, GL_DIFFUSE, pointLightDiffuse);   // Propriedade difusa
    glLightfv(GL_LIGHT1, GL_SPECULAR, pointLightSpecular); // Propriedade especular
    glLightfv(GL_LIGHT1, GL_POSITION, pointLightPosition); // Posição da luz

    // Ativando a iluminação e GL_LIGHT0
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
}

void setMaterialProperties(GLfloat ambient[], GLfloat diffuse[], GLfloat specular[], GLfloat shininess) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Configurar a câmera
    gluLookAt(p.position[X], p.position[Y], p.position[Z],  // Posição da câmera (player)
              p.lookAt[X], p.lookAt[Y], 0.0,    // Para onde a câmera olha
              0.0, 1.0, 0.0);

    GLfloat ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat diffuse[] = { 0.2, 0.8, 0.2, 1.0 };
    GLfloat specular[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat shininess = 10.0;
    setMaterialProperties(ambient, diffuse, specular, shininess);

    // Desenhar o chão
    glBegin(GL_QUADS);
        glVertex3f(-20.0f, -10.0f, -20.0f);
        glVertex3f(20.0f, -10.0f, -20.0f);
        glVertex3f(20.0f, -10.0f, 100.0f);
        glVertex3f(-20.0f, -10.0f, 100.0f);
    glEnd();

    // Desenhar o teto
    glBegin(GL_QUADS);
        glVertex3f(-20.0f, 20.0f, -20.0f);
        glVertex3f(20.0f, 20.0f, -20.0f);
        glVertex3f(20.0f, 20.0f, 100.0f);
        glVertex3f(-20.0f, 20.0f, 100.0f);
    glEnd();

    GLfloat ambient3[] = { 0.6, 0.2, 0.2, 1.0 };
    GLfloat diffuse3[] = { 0.6, 0.2, 0.2, 1.0 };
    GLfloat specular3[] = { 0.1, 0.1, 0.1, 1.0 };
    setMaterialProperties(ambient3, diffuse3, specular3, shininess);

    // Desenhar parede frente
    glBegin(GL_QUADS);
        glVertex3f(-20.0f, 20.0f, -20.0f);
        glVertex3f(20.0f, 20.0f, -20.0f);
        glVertex3f(20.0f, -10.0f, -20.0f);
        glVertex3f(-20.0f, -10.0f, -20.0f);
    glEnd();

    // Desenhar parede esq
    glBegin(GL_QUADS);
        glVertex3f(-20.0f, 20.0f, -20.0f);
        glVertex3f(-20.0f, -10.0f, -20.0f);
        glVertex3f(-20.0f, -10.0f, 100.0f);
        glVertex3f(-20.0f, 20.0f, 100.0f);
    glEnd();

    // Desenhar parede dir
    glBegin(GL_QUADS);
        glVertex3f(20.0f, 20.0f, -20.0f);
        glVertex3f(20.0f, -10.0f, -20.0f);
        glVertex3f(20.0f, -10.0f, 100.0f);
        glVertex3f(20.0f, 20.0f, 100.0f);
    glEnd();

    printEnemies(enemies);

    GLfloat ambient2[] = { 0.2, 0.5, 0.8, 1.0 };
    GLfloat diffuse2[] = { 0.2, 0.5, 0.8, 1.0 };
    GLfloat specular2[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat shininess2 = 10.0;
    setMaterialProperties(ambient, diffuse2, specular2, shininess2);

    drawObject();

    renderText(640.0f, 360.0f, "+");

    glFlush();
    glutSwapBuffers();
}

void update(int value) {
    if(actualFrame < fireRating) {
        actualFrame++;
    } else if (actualFrame == fireRating) shotAnim = 0;

    updateEnemies(&enemies);

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // aproximadamente 60 FPS (1000ms/16ms = 62.5 updates por segundo)
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0); // Cor de fundo
    glEnable(GL_DEPTH_TEST); // Ativar teste de profundidade

    // Configurar a proje��o
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT, 1.0, 100.0);

    // Configurar a visualiza��o do modelo
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    initLighting();
}


void keypress(int key, int x, int y) {
    switch (key) {
        case 27:
            if(showCursor) {
                glutSetCursor(GLUT_CURSOR_NONE);
                showCursor = 0;
            } else {
                glutSetCursor(GLUT_CURSOR_INHERIT);
                showCursor = 1;
            }
        break;
    }
}

void mouseMotion(int x, int y) {
    if (m.x == -1 || m.y == -1) {
        m.x = x;
        m.y = y;
    }

    if(x >= 1180) {
        SetCursorPos(glutGet(GLUT_WINDOW_X) + 640, y + glutGet(GLUT_WINDOW_Y));
    } else if (x <= 100) {
        SetCursorPos(glutGet(GLUT_WINDOW_X) + 640, y + glutGet(GLUT_WINDOW_Y));
    }

    if(y > 620) {
        SetCursorPos(glutGet(GLUT_WINDOW_X) + x, glutGet(GLUT_WINDOW_Y) + 360);
    } else if (y <= 100) {
        SetCursorPos(glutGet(GLUT_WINDOW_X) + x, glutGet(GLUT_WINDOW_Y) + 360);
    }

    // Determinar direção no eixo X
    if (x > m.x) {
        p.lookAt[0] += p.sensibility;
        rotationY -= p.sensibility;
    } else if (x < m.x) {
        p.lookAt[0] -= p.sensibility;
        rotationY += p.sensibility;
    }

    // Determinar direção no eixo Y
    if (y > m.y) {
        p.lookAt[1] -= p.sensibility;
        rotationZ += p.sensibility;
    } else if (y < m.y) {
        p.lookAt[1] += p.sensibility;
        rotationZ -= p.sensibility;
    }

    // Atualizar última posição do mouse
    m.x = x;
    m.y = y;
}

void mouseClick(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        switch (button) {
        case GLUT_LEFT_BUTTON:
            if(actualFrame == fireRating) {
                actualFrame = 0;
                shotAnim = 1;
            }
            break;
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    int window = glutCreateWindow("FPS daora");
    glutSetWindow(window);
    //glutFullScreen();
    init();
    initPlayer();
    initEnemies();

    glutSetCursor(GLUT_CURSOR_NONE);
    loadObject("pistolinha.obj");

    glutDisplayFunc(display);
    glutKeyboardFunc(keypress);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(mouseClick);
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}
