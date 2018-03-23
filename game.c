#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define DEBUG 0
#define DOIT 0
#define WIDTH 40
#define HEIGHT 20

typedef struct {
    int x, y;
    int parts[6][2];
    short partAmt;
    short rotation;
    short canRotate;
} object;

void moveSide(char* mapa, object* peca, int side);

void clearArray(char* array, int length, int fill);

void doGameTick(char* mapa, object* peca);

void updateScreen(char* mapa);

object randomObject(char* mapa, int x, int y, int count);

int canGoXY(char* mapa ,object* peca, int x, int y);

void createObject(char* mapa, object* peca);

void eraseObject(char* mapa, object* peca);

void cx(int *x);

void cy(int *y);

int vx(int x);

int vy(int y);

int cX(int x);

int cY(int y);

char getch();

int kbhit();

int main() {
    srand(time(NULL));
    system("clear");

    char* mapa = malloc(WIDTH * HEIGHT);

    clearArray(mapa, WIDTH * HEIGHT, ' ');

    int running = 1;

    clock_t delay = CLOCKS_PER_SEC/3;

    object peca = randomObject(mapa, -1, 0, 0);

    createObject(mapa, &peca);
    updateScreen(mapa);
    clock_t delta = 0;
    clock_t lastTime = clock(), curTime = 0;
    
    while(running) {
        curTime = clock();
        delta += curTime - lastTime;

        if(kbhit()) {
            char in = getch();
            while(kbhit()) getch();
            if(in == 'x' || in == 'X')
                running = 0;
            else if(in == 'b' || in == 'B')
                delta += CLOCKS_PER_SEC;
            else if(in == 'a' || in == 'A') {
                moveSide(mapa, &peca, -1);
            }
            updateScreen(mapa);
        }

        if(delta >= delay) {
            delta -= delay;
            doGameTick(mapa, &peca);
            updateScreen(mapa);
        }

        lastTime = curTime;
    }
    free(mapa);
    return 0;
}

void moveSide(char* mapa, object* peca, int side) {
    eraseObject(mapa, peca);
    
}

void clearArray(char* array, int length, int fill) {
    while(length--) {
        array[length] = fill;
    }
}

void doGameTick(char* mapa, object* peca) {
    eraseObject(mapa, peca);
    if(canGoXY(mapa, peca)) {
        int i = 0;
        peca->y++;
        cy(&peca->y);
        for(;i < peca->partAmt; i++)
            peca->parts[i][1]++;
            cy(&peca->parts[i][1]);
        createObject(mapa, peca);
    } else {
        createObject(mapa, peca);
        *peca = randomObject(mapa, -1, 0, 0);
        createObject(mapa, peca);
    }
}

int canGoXY(char* mapa ,object* peca, int x, int y) {
    int i = 0;
    for(;i < peca->partAmt;i++) {
        if( mapa[ (peca->parts[i][1] + y) * WIDTH + peca->parts[i][0] + x] != ' '
         || !vx(peca->parts[i][0] + x) || !vy(peca->parts[i][1] + y) )
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

object randomObject(char* mapa, int x, int y, int count) {
    if(x == -1) {
        x = WIDTH / 2;
        y = 0;
    }

    int r = rand() % 4;
    object peca = {.x = cX(x), .y = cY(y), .rotation = 0, .partAmt = 3, .parts = {}};
    if(count >= 10000) {
        printf("deu merda na random object");
        exit(1);
    }
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
    int i = 0;
    for(;i < peca.partAmt;i++) {
        if(!vx(peca.parts[i][0]) || !vy(peca.parts[i][1]) || mapa[peca.parts[i][1] * WIDTH + peca.parts[i][0]] != ' ') {
            return randomObject(mapa, x, y, count + 1);
        }
    }

    return peca;
}
void cx(int *x) {
    *x = cX(*x);
}

void cy(int *y) {
    *y = cY(*y);
}

int vx(int x) {
    return cX(x) == x;
}

int vy(int y) {
    return cY(y) == y;
}

int cX(int x) {
    return x >= WIDTH ? WIDTH - 1 : x < 0 ? 0 : x;
}

int cY(int y) {
    return y >= HEIGHT ? HEIGHT - 1 : y < 0 ? 0 : y;
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
