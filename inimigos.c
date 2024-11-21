#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "inimigos.h"

enemy randEnemy() {
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

    enemy e = { {x, y, z},
                {colorR, colorG, colorB},
                0.1f};

    return e;
}

Enemy* createEnemy(enemy e) {
    Enemy* newEnemy = (Enemy*)malloc(sizeof(Enemy));
    if (!newEnemy) {
        printf("Erro ao alocar memoria!\n");
        exit(1);
    }
    newEnemy->data = e;
    newEnemy->next = NULL;
    return newEnemy;
}

Enemy* addEnemy(Enemy* head, enemy e) {
    Enemy* newEnemy = createEnemy(e);
    if (!head) {
        return newEnemy;
    }
    Enemy* temp = head;
    while (temp->next) {
        temp = temp->next;
    }
    temp->next = newEnemy;
    return head;
}

void printEnemies(Enemy* head) {
    if (!head) {
        return;
    }
    Enemy* temp = head;
    while (temp) {
        enemy e = temp->data;

        GLfloat ambient[] = { e.cor[0], e.cor[1], e.cor[2] };
        GLfloat diffuse[] = { e.cor[0], e.cor[1], e.cor[2] };
        GLfloat specular[] = { 0.1, 0.1, 0.1, 1.0 };
        GLfloat shininess = 10.0;

        glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT, GL_SHININESS, shininess);
        
        glPushMatrix(); // Salva a matriz de transformação atual
        glTranslatef(e.pos[0], e.pos[1], e.pos[2]); // Move para a posição desejada
        glutSolidSphere(2, 20, 20); // Desenha a esfera sólida
        glPopMatrix(); // Restaura a matriz de transformação

        temp = temp->next;
    }
}

void updateEnemies(Enemy** head) {
    if (!head || !*head) {
        return;
    }

    Enemy* temp = *head;
    Enemy* prev = NULL;

    while (temp) {
        if (temp->data.pos[2] >= 30) {
            if (prev == NULL) {
                *head = temp->next;
            } else {
                prev->next = temp->next;
            }

            Enemy* toDelete = temp;
            temp = temp->next;
            free(toDelete);
        } else {
            temp->data.pos[2] = temp->data.pos[2] + temp->data.speed;
            prev = temp;
            temp = temp->next;
        }
    }
}