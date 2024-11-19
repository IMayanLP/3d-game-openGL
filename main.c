#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define MAX_VERTICES 10000
#define MAX_FACES 10000

enum Coords {X, Y, Z};

//teste

typedef struct {
    float x, y, z;
} Vertice3D;

typedef struct {
    float u, v;
} Textura;

typedef struct {
    float x, y, z;
} Normal;

typedef struct {
    int v[4];  // Índices dos vértices
    int t[4];  // Índices das texturas
    int n[4];  // Índices das normais
} Face;

Vertice3D vertices[MAX_VERTICES];
Textura texturas[MAX_VERTICES];
Normal normais[MAX_VERTICES];
Face faces[MAX_FACES];
int numVertices = 0;
int numTexturas = 0;
int numNormais = 0;
int numFaces = 0;

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

Player p;
Mouse m;
int showCursor = 0;
GLfloat rotationY = -90.0f;
GLfloat rotationZ = -5.0f;

void loadObject(const char *nomeArquivo) {
    FILE *fp = fopen(nomeArquivo, "r");
    if (!fp) {
        printf("Erro ao abrir o arquivo %s\n", nomeArquivo);
        exit(1);
    }

    char linha[128];
    while (fgets(linha, sizeof(linha), fp)) {
        if (strncmp(linha, "v ", 2) == 0) {
            Vertice3D vertice;
            sscanf(linha, "v %f %f %f", &vertice.x, &vertice.y, &vertice.z);
            vertices[numVertices++] = vertice;
        } else if (strncmp(linha, "vt ", 3) == 0) {
            Textura texCoord;
            sscanf(linha, "vt %f %f", &texCoord.u, &texCoord.v);
            texturas[numTexturas++] = texCoord;
        } else if (strncmp(linha, "vn ", 3) == 0) {
            Normal normal;
            sscanf(linha, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
            normais[numNormais++] = normal;
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
            faces[numFaces++] = face;
        }
    }
    fclose(fp);
    printf("Carregado %d vértices, %d texturas, %d normais e %d faces do arquivo %s\n",
           numVertices, numTexturas, numNormais, numFaces, nomeArquivo);
}

void drawObject() {
    glPushMatrix(); // Salva a matriz de transformação atual

    // Passo 2: Transladar o sistema de coordenadas para que a posição da câmera seja a origem
    glTranslatef(p.position[X], p.position[Y] - 5, p.position[Z] - 15);

    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(rotationZ, 0.0f, 0.0f, 1.0f);

    for (int i = 0; i < numFaces; i++) {
        Face face = faces[i];
        glBegin(face.v[3] == -1 ? GL_TRIANGLES : GL_QUADS);
        for (int j = 0; j < 4 && face.v[j] != -1; j++) {
            if (face.n[j] != -1) {  // Aplica a normal
                Normal normal = normais[face.n[j]];
                glNormal3f(normal.x, normal.y, normal.z);
            }
            if (face.t[j] != -1) {  // Aplica a coordenada de textura
                Textura texCoord = texturas[face.t[j]];
                glTexCoord2f(texCoord.u, texCoord.v);
            }
            Vertice3D vertice = vertices[face.v[j]];
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

    // Configuração da GL_LIGHT1 (luz pontual em uma posição XYZ específica)
    GLfloat pointLightDiffuse[] = { 0.2f, 0.2f, 0.2f, 1.0f }; // Cor difusa (branca)
    GLfloat pointLightSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f }; // Reflexão especular (branca)
    GLfloat pointLightPosition[] = { 0.0f, 19.0f, 25.0f, 1.0f }; // Posição XYZ da luz pontual

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
              0.0, 1.0, 0.0);   // Vetor "up"

    // Configurar material para o chão
    GLfloat ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat diffuse[] = { 0.2, 0.8, 0.2, 1.0 };
    GLfloat specular[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat shininess = 10.0;
    setMaterialProperties(ambient, diffuse, specular, shininess);

    // Desenhar o chão menor
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

    GLfloat ambient3[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat diffuse3[] = { 0.8, 0.2, 0.2, 1.0 };
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

    GLfloat ambient2[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat diffuse2[] = { 0.0, 0.5, 0.8, 1.0 };
    GLfloat specular2[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat shininess2 = 10.0;
    setMaterialProperties(ambient, diffuse2, specular2, shininess2);

    drawObject();

    glFlush();
    glutSwapBuffers();
}

void update(int value) {

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0); // Cor de fundo
    glEnable(GL_DEPTH_TEST); // Ativar teste de profundidade

    // Configurar a proje��o
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 1.0, 100.0);

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

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    int window = glutCreateWindow("FPS daora");
    glutSetWindow(window);
    //glutFullScreen();
    init();
    initPlayer();

    glutSetCursor(GLUT_CURSOR_NONE);
    loadObject("pistolinha.obj");

    glutDisplayFunc(display);
    glutKeyboardFunc(keypress);
    glutPassiveMotionFunc(mouseMotion); // função do mouse
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}
