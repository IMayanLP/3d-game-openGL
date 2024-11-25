typedef struct {
    GLfloat pos[3];
    float raio;
    GLfloat cor[3];
    GLfloat speed;
} enemy;

typedef struct Enemy {
    enemy data;
    int ativo;
    struct Enemy* next;
} Enemy;

Enemy* createEnemy(enemy);
Enemy* addEnemy(Enemy*, enemy);
void printEnemies(Enemy*);
void updateEnemies(Enemy**, int*);
enemy randEnemy();