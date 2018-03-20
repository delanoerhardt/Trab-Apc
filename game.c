#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define DEBUG 0
#define WIDTH 40
#define HEIGHT 20

typedef struct {
    short x, y;
    short parts[6][2];
    short partAmt;
    short rotation;
    short canRotate;
} object;

void clearArray(char* array, int length, int fill);

void doGameTick(char* mapa, object* peca);

void updateScreen(char* mapa);

object randomObject(int x, int y);

int canGoDown(char* mapa ,object* peca);

void createObject(char* mapa, object* peca);

void eraseObject(char* mapa, object* peca);

char getch();

int kbhit();

int main() {
    srand(time(NULL));
    system("clear");

    char* mapaMatrix = malloc(WIDTH * HEIGHT);

    clearArray(mapaMatrix, WIDTH * HEIGHT, ' ');

    int running = 1;

    clock_t delay = CLOCKS_PER_SEC;

    object peca = randomObject(0, 0);

    clock_t delta = 0;
    clock_t lastTime = clock(), curTime = 0;
    while(running) {
        curTime = clock();
        delta += curTime - lastTime;

        if(kbhit()) {
            char in = getch();
            if(in == 'a' || in == 'A')
                running = 0;
            updateScreen(mapaMatrix);
        }

        if(delta >= delay) {
            delta -= delay;
            doGameTick(mapaMatrix, &peca);
            updateScreen(mapaMatrix);
        }

        lastTime = curTime;
    }
    return 0;
}

void clearArray(char* array, int length, int fill) {
    while(length--) {
        array[length] = fill;
    }
}

void doGameTick(char* mapa, object* peca) {
    eraseObject(mapa, peca);
    if(canGoDown(mapa, peca)) {
        int i = 0;
        peca->y++;
        for(;i < peca->partAmt; i++)
            peca->parts[i][1]++;
    }
    createObject(mapa, peca);
}

int canGoDown(char* mapa ,object* peca) {
    int i = 0;
    for(;i < peca->partAmt;i++) {
        if(mapa[peca->parts[i][1] * WIDTH + peca->parts[i][0]] != ' ')
            return 0;
    }
    return 1;
}

void createObject(char* mapa, object* peca) {
    int i = 0;
    for(;i < peca->partAmt;i++) {
        mapa[peca->parts[i][1] * WIDTH + peca->parts[i][0]] = '@';
    }
}

void eraseObject(char* mapa, object* peca) {
    int i = 0;
    for(;i < peca->partAmt;i ++) {
        mapa[peca->parts[i][1] * WIDTH + peca->parts[i][0]] = ' ';
    }
}

void updateScreen(char* mapa) {
    system("clear");
    int i = 0;
    for(;i < WIDTH + 2;i++) {
        putchar('#');
    }
    putchar('\n');
    for(i = 0;i < HEIGHT; i++) {
        putchar('#');
        printf("%.*s", WIDTH, mapa + (i * WIDTH));
        printf("#\n");
    }
    for(i = 0;i < WIDTH + 2;i++) {
        putchar('#');
    }
}

object randomObject(int x, int y) {
    if(x == -1) x = WIDTH / 2, y = 2;
    int r = rand() % 4;
    object peca = {.x = x, .y = y, .rotation = 0, .partAmt = 3, .parts = {}};
    switch(r) {
        case 0:
            peca.partAmt = 4;
            peca.parts[0][0] = x;
            peca.parts[0][1] = y;
            peca.parts[1][0] = x + 1;
            peca.parts[1][1] = y;
            peca.parts[2][0] = x + 1;
            peca.parts[2][1] = y + 1;
            peca.parts[3][0] = x;
            peca.parts[3][1] = y + 1;
            peca.canRotate = 0;
            break;
        case 1:
            peca.partAmt = 4;
            peca.parts[0][0] = x;
            peca.parts[0][1] = y;
            peca.parts[1][0] = x - 1;
            peca.parts[1][1] = y;
            peca.parts[2][0] = x;
            peca.parts[2][1] = y + 1;
            peca.parts[3][0] = x + 1;
            peca.parts[3][1] = y + 1;
            peca.canRotate = 1;
            break;
        case 2:
            peca.partAmt = 4;
            peca.parts[0][0] = x;
            peca.parts[0][1] = y;
            peca.parts[1][0] = x + 1;
            peca.parts[1][1] = y;
            peca.parts[2][0] = x;
            peca.parts[2][1] = y + 1;
            peca.parts[3][0] = x - 1;
            peca.parts[3][1] = y + 1;
            peca.canRotate = 1;
            break;
        case 3:
            peca.partAmt = 4;
            peca.parts[0][0] = x;
            peca.parts[0][1] = y;
            peca.parts[1][0] = x - 1;
            peca.parts[1][1] = y;
            peca.parts[2][0] = x + 1;
            peca.parts[2][1] = y;
            peca.parts[3][0] = x;
            peca.parts[3][1] = y + 1;
            peca.canRotate = 1;
            break;
    }
    return peca;
}

char getch()
{
    int ch;
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO,&oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}