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
    short canRotate;
} object;

void toLowerPos(char* mapa, object* pecaFantasma);

void toBottom(char* mapa, object* peca);

void copyObject(object* peca, object peca2);

void turnPiece(char* mapa, object* peca, int direction);

void matrixProduct(int* a, int* b, int aX, int aY, int bX);

int moveToXY(char* mapa, object* peca, int x, int y);

void clearArray(char* array, int length, int fill);

void verifyLines(char* mapa, object* peca);

void doGameTick(char* mapa, object* peca);

void updateScreen(char* mapa, object pecaFantasma);

object randomObject(char* mapa, int x, int y, int count);

int canGoXY(char* mapa ,object* peca, int x, int y);

void createObject(char* mapa, object* peca);

void eraseObject(char* mapa, object* peca);

int vp(int parts[2], int x, int y);

int v(int x, int y);

void cx(int *x);

void cy(int *y);

int vx(int x);

int vy(int y);

int cX(int x);

int cY(int y);

char getch();

int kbhit();

int score = 0;

int main() {
    srand(time(NULL));
    system("clear");

    char* mapa = malloc(WIDTH * HEIGHT);

    clearArray(mapa, WIDTH * HEIGHT, ' ');

    int running = 1;

    clock_t delay = CLOCKS_PER_SEC;

    object peca = randomObject(mapa, -1, 0, 0);

    createObject(mapa, &peca);
    object pecaFantasma = peca;
    toLowerPos(mapa, &pecaFantasma);
    updateScreen(mapa, pecaFantasma);
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
            else if(in == 'a' || in == 'A')
                moveToXY(mapa, &peca, -1, 0);
            else if(in == 'd' || in == 'D')
                moveToXY(mapa, &peca, +1, 0);
            else if(in == 's' || in == 'S') {
                doGameTick(mapa, &peca);
                delta /= 3;
            }
            else if(in == 'w' || in == 'W')
                turnPiece(mapa, &peca, 1);
            else if(in == ' ') {
                if(moveToXY(mapa, &peca, 0, +1)) {
                    toBottom(mapa, &peca);
                    delta = 0;
                }
            }
            pecaFantasma = peca;
            toLowerPos(mapa, &pecaFantasma);
            updateScreen(mapa, pecaFantasma);
        }

        if(delta >= delay) {
            delta -= delay;
            doGameTick(mapa, &peca);
            pecaFantasma = peca;
            toLowerPos(mapa, &pecaFantasma);
            updateScreen(mapa, pecaFantasma);
        }

        lastTime = curTime;
    }
    free(mapa);
    return 0;
}

void toLowerPos(char* mapa, object* pecaFantasma) {
    while(canGoXY(mapa, pecaFantasma, 0, +1)) {
        pecaFantasma->y++;
    }
}

void toBottom(char* mapa, object* peca) {
    while(moveToXY(mapa, peca, 0, +1)) {}
}

void copyObject(object* peca, object peca2) {
    peca->x = peca2.x;
    peca->y = peca2.y;
    peca->partAmt = peca2.partAmt;
    peca->canRotate = peca2.canRotate;
    int i = 0;
    for(;i < peca->partAmt; i++) {
        peca->parts[i][0] = peca2.parts[i][0];
        peca->parts[i][1] = peca2.parts[i][1];
    }
}

void turnPiece(char* mapa, object* peca, int direction) {
    if(peca->canRotate == 0) return;
    eraseObject(mapa, peca);
    object test = *peca;
    int fund[2 * 2] = {0, direction, -direction, 0};
    matrixProduct(&test.parts[0][0], fund, 2, test.partAmt, 2);
    if(canGoXY(mapa, &test, 0, 0)) {
        copyObject(peca, test);
    }
    createObject(mapa, peca);
}

void matrixProduct(int* a, int* b, int aX, int aY, int bX) {
    int* holder = malloc(aY * aX * sizeof(int));
    int i = 0, j = 0;
    for(;i < aY; i++) {
        for(j = 0; j < aX; j++) {
            holder[i * aX + j] = (a[i * aX + 0] * b[j]) + (a[i * aX + 1] * b[aX + j]);
        }
    }
    for(i = 0;i < aY * aX;i++) {
        a[i] = holder[i];
    }
    free(holder);
}

int moveToXY(char* mapa, object* peca, int x, int y) {
    int moved = 0;
    eraseObject(mapa, peca);
    if(canGoXY(mapa, peca, x, y)) {
        peca->x += x;
        peca->y += y;
        cx(&peca->x);
        cy(&peca->y);
        moved = 1;
    }
    createObject(mapa, peca);
    return moved;
}

void clearArray(char* array, int length, int fill) {
    while(length--) {
        array[length] = fill;
    }
}

void doGameTick(char* mapa, object* peca) {
    if( !moveToXY(mapa, peca, 0, +1) ) {
        verifyLines(mapa, peca);
        *peca = randomObject(mapa, -1, 0, 0);
        createObject(mapa, peca);
    }
}

void verifyLines(char* mapa, object* peca) {
    int i = HEIGHT - 1, j = 0, k = 0, scoreMultiplier = 1;
    for(;i >= 0; i--) {
        for(j = 0, k = 0;j < WIDTH;j++) {
            k += mapa[i * WIDTH + j] == '@';
        }
        if(k == WIDTH) {
            for(j = 0;j < WIDTH;j++) {
                mapa[i * WIDTH + j] = ' ';
            }
            for(j = i - 1; j >= 0; j--) {
                for(k = 0; k < WIDTH; k++) {
                    mapa[(j + 1) * WIDTH + k] = mapa[j * WIDTH + k];
                }
            }
            score += 10 * scoreMultiplier;
            scoreMultiplier *= 2;
            i++;
        }
    }
}

int canGoXY(char* mapa ,object* peca, int x, int y) {
    int i = 0;
    for(;i < peca->partAmt;i++) {
        if( !v(peca->parts[i][0] + peca->x + x,peca->parts[i][1] + peca->y + y) || mapa[ (peca->parts[i][1] + peca->y + y) * WIDTH + peca->parts[i][0] + peca->x + x] != ' ')
            return 0;
    }
    return 1;
}

void createObject(char* mapa, object* peca) {
    int i = 0;
    for(;i < peca->partAmt;i++) {
        mapa[(peca->parts[i][1] + peca->y) * WIDTH + (peca->parts[i][0] + peca->x)] = '@';
    }
}

void eraseObject(char* mapa, object* peca) {
    int i = 0;
    for(;i < peca->partAmt;i ++) {
        mapa[(peca->parts[i][1] + peca->y) * WIDTH + (peca->parts[i][0] + peca->x)] = ' ';
    }
}

void updateScreen(char* mapa, object pecaFantasma) {
    system("clear");
    int i = 0;
    for(;i < WIDTH + 2;i++) {
        putchar('#');
    }
    printf("  Score: %d\n", score);
    for(i = 0;i < HEIGHT; i++) {
        putchar('#');
        printf("%.*s", WIDTH, mapa + (i * WIDTH));
        printf("#\n");
    }
    for(i = 0;i < WIDTH + 2;i++) {
        char a = 0;
        if(i - 1 >= 0 && i - 1 < WIDTH) {
            int j = 0;
            for(;j < pecaFantasma.partAmt;j++) {
                if(pecaFantasma.x + pecaFantasma.parts[j][0] == i - 1) {
                    printf("\x1B[7m");
                    a = 1;
                    break;
                }
            }
        }
        putchar('#');
        if(a) printf("\x1B[0m");
    }
}

object randomObject(char* mapa, int x, int y, int count) {
    if(x == -1) {
        x = WIDTH / 2;
        y = 0;
    }

    int r = rand() % 7;
    object peca = {.x = cX(x), .y = cY(y), .partAmt = 3, .parts = {}};
    if(count >= 10000) {
        printf("Se fodeu");
        exit(1);
    }
    switch(r) {
        case 0:
            peca.partAmt = 4;
            peca.parts[0][0] = 0;
            peca.parts[0][1] = 0;
            peca.parts[1][0] = +1;
            peca.parts[1][1] = 0;
            peca.parts[2][0] = +1;
            peca.parts[2][1] = +1;
            peca.parts[3][0] = 0;
            peca.parts[3][1] = +1;
            peca.canRotate = 0;
            break;
        case 1:
            peca.partAmt = 4;
            peca.parts[0][0] = 0;
            peca.parts[0][1] = 0;
            peca.parts[1][0] = -1;
            peca.parts[1][1] = 0;
            peca.parts[2][0] = 0;
            peca.parts[2][1] = +1;
            peca.parts[3][0] = +1;
            peca.parts[3][1] = +1;
            peca.canRotate = 1;
            break;
        case 2:
            peca.partAmt = 4;
            peca.parts[0][0] = 0;
            peca.parts[0][1] = 0;
            peca.parts[1][0] = +1;
            peca.parts[1][1] = 0;
            peca.parts[2][0] = 0;
            peca.parts[2][1] = +1;
            peca.parts[3][0] = -1;
            peca.parts[3][1] = +1;
            peca.canRotate = 1;
            break;
        case 3:
            peca.partAmt = 4;
            peca.parts[0][0] = 0;
            peca.parts[0][1] = 0;
            peca.parts[1][0] = -1;
            peca.parts[1][1] = 0;
            peca.parts[2][0] = +1;
            peca.parts[2][1] = 0;
            peca.parts[3][0] = 0;
            peca.parts[3][1] = +1;
            peca.canRotate = 1;
            break;
        case 4:
            peca.partAmt = 4;
            peca.parts[0][0] = 0;
            peca.parts[0][1] = 0;
            peca.parts[1][0] = 0;
            peca.parts[1][1] = +1;
            peca.parts[2][0] = +1;
            peca.parts[2][1] = 0;
            peca.parts[3][0] = +2;
            peca.parts[3][1] = 0;
            peca.canRotate = 1;
            break;
        case 5:
            peca.partAmt = 4;
            peca.parts[0][0] = 0;
            peca.parts[0][1] = 0;
            peca.parts[1][0] = 0;
            peca.parts[1][1] = +1;
            peca.parts[2][0] = -1;
            peca.parts[2][1] = 0;
            peca.parts[3][0] = -2;
            peca.parts[3][1] = 0;
            peca.canRotate = 1;
            break;
        case 6:
            peca.partAmt = 5;
            peca.parts[0][0] = 0;
            peca.parts[0][1] = 0;
            peca.parts[1][0] = -1;
            peca.parts[1][1] = 0;
            peca.parts[2][0] = -2;
            peca.parts[2][1] = 0;
            peca.parts[3][0] = +1;
            peca.parts[3][1] = 0;
            peca.parts[4][0] = +2;
            peca.parts[4][1] = 0;
            peca.canRotate = 1;
            break;
    }
    int i = 0;
    for(;i < peca.partAmt;i++) {
        if(!vp(peca.parts[i], peca.x, peca.y) || mapa[(peca.parts[i][1] + peca.y) * WIDTH + (peca.parts[i][0] + peca.x) ] != ' ') {
            return randomObject(mapa, x, y, count + 1);
        }
    }

    return peca;
}

int vp(int parts[2], int x, int y) {
    return v( x + parts[0], y + parts[1] );
}

int v(int x, int y) {
    return vx( x ) && vy( y );
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
