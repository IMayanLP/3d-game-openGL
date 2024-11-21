typedef struct {
    GLfloat pos[3];
    GLfloat cor[3];
    GLfloat speed;
} enemy;

typedef struct Enemy {
    enemy data;
    struct Enemy* next;
} Enemy;

Enemy* createEnemy(enemy);
Enemy* addEnemy(Enemy*, enemy);
void printEnemies(Enemy*);
void updateEnemies(Enemy**);
enemy randEnemy();